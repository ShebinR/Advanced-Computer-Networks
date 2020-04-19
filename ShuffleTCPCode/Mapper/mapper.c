#include <stdio.h> 
#include <string.h> 
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h> 
 
#include "constants.h"
#include "hash_map.h"
#include "tcp_server.h"
#include "communication.h"
#include "thread_info_mapper.h"

void rand_string(char *str, size_t size)
{
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJK";
    if (size) {
        --size;
        for (size_t n = 0; n < size; n++) {
            int key = rand() % (int) (sizeof charset - 1);
            str[n] = charset[key];
        }
        str[size] = '\0';
    }
}

hash_map * generateData() {
    hash_map *map = createHashMap(HASH_MAP_SIZE);

    for(int i = 0; i < MAX_RECORDS; i++) {
        char *key = (char *) malloc(sizeof(char) * KEY_SIZE);
        rand_string(key, KEY_SIZE);
        char *val = (char *) malloc(sizeof(char) * VAL_SIZE);
        rand_string(val, VAL_SIZE);
        insert(map, key, val);

        free(key); free(val);
    }

    //printMap(map);

    return map;
}

char** getChunk(hash_map_iterator *itr, int count, int *record_count) {
    int record = 0;
    char **strings = (char **) malloc (sizeof(char *) * (count * 2));

    for(record = 0; record != (count * 2); record += 2) {
        char *key = (char *) malloc(sizeof(char) * KEY_SIZE);
        char *value = (char *) malloc(sizeof(char) * VAL_SIZE);
        int ret = getNext(itr, key, value);
        if(ret == 0) {
            //printf("KEY : %s\n", key);
            //printf("VALUE : %s\n", value);
            strings[record] = key;
            strings[record + 1] = value;
        } else {
          //printf("REACHED END!\n");
            break;
        }
    }
    if(record == 0)
        return NULL;
    *record_count = record;
    return strings;
}

int getRequestVal(pthread_mutex_t *lock, int *var) {
    pthread_mutex_lock(lock);
    int req_val = *var;
    pthread_mutex_unlock(lock);
    return req_val;
}

void updateRequestVal(pthread_mutex_t *lock, int *var, int update) {
    pthread_mutex_lock(lock);
    *var = *var + update;
    pthread_mutex_unlock(lock);
}

void receiverThread(void *input) {
    pthread_mutex_t *request_update_lock = ((receiver_thread_info *)input)->request_update_lock;
    int *pending_request = ((receiver_thread_info *)input)->pending_request;
    int connfd = ((receiver_thread_info *)input)->connfd;
    int *last_request = ((receiver_thread_info *)input)->last_request;
    int total_shuffle_size = ((receiver_thread_info *)input)->total_shuffle_size;
   
    int reqs_received = 0;
    while(1) {
        int ret = receiveChunckFetchRequest(connfd);
        if(ret == -1) {
            printf("ERROR: receiving chunck_fetch_request()");
        }
        reqs_received++;
        updateRequestVal(request_update_lock, pending_request, 1);
        printf("RECEIVER: Received a packet!\n");
        if(reqs_received == total_shuffle_size) {
            printf("RECEIVER: *******Last request received!********\n");
            *last_request = 1; 
            break;
        }
    }
}

void senderThread(void *input) {
    pthread_mutex_t *request_update_lock = ((sender_thread_info *)input)->request_update_lock;
    int *pending_request = ((sender_thread_info *)input)->pending_request;
    int connfd = ((sender_thread_info *)input)->connfd;
    int per_chunck_record = ((sender_thread_info *)input)->per_chunck_record;
    hash_map_iterator *itr = ((sender_thread_info *)input)->itr;
    int *last_request = ((sender_thread_info *)input)->last_request;
    int total_shuffle_size = ((sender_thread_info *)input)->total_shuffle_size;

    int num_responses = 0;
    while(1) {
        int curr_req_val = getRequestVal(request_update_lock, pending_request);
        if(*last_request == 1 && curr_req_val == 0) {
            printf("SENDER: reached end of shuffle!\n");
            break;
        }
        if(curr_req_val == 0) {
            printf("SENDER: No request yet!\n");
            usleep(1000000);
            continue;
        }
        printf("SENDER: Got a request!\n");
        updateRequestVal(request_update_lock, pending_request, -1);

        int record_count = 0;
        char **chunck = getChunk(itr, per_chunck_record, &record_count);
        if(chunck == NULL) {
            printf("INFO: No more data to push out!\n");
            // Send end of record send??
            break;
        }
        sendChunckFetchReply(connfd, chunck, record_count);
        num_responses++;

        for(int k = 0; k < per_chunck_record * 2; k++) {
            //printf("%d : %s\n", k, chunck[k]);
            free(chunck[k]);
        }
        free(chunck);
    }
}

void startShuffleThreads(int connfd, hash_map *map, int total_shuffle_size) {
    //printMap(map);
    hash_map_iterator *itr = createIterator(map);

    pthread_mutex_t request_update_lock;
    if(pthread_mutex_init(&(request_update_lock), NULL) != 0) {
        printf("ERROR: Queue mutex init has failed\n");
        return;
    }
    int pending_request = 0;
    int last_request = 0;
    int per_chunck_record = MAX_REPLY_SIZE;

    receiver_thread_info *rcvr = (receiver_thread_info *) malloc (sizeof(receiver_thread_info));
    rcvr->request_update_lock = &request_update_lock;
    rcvr->pending_request = &pending_request;
    rcvr->connfd = connfd;
    rcvr->last_request = &last_request;
    rcvr->total_shuffle_size = total_shuffle_size;

    sender_thread_info *sndr = (sender_thread_info *) malloc (sizeof(sender_thread_info));
    sndr->request_update_lock = &request_update_lock;
    sndr->pending_request = &pending_request;
    sndr->connfd = connfd;
    sndr->per_chunck_record = per_chunck_record;
    sndr->itr = itr;
    sndr->last_request = &last_request;
    sndr->total_shuffle_size = total_shuffle_size;

    pthread_t receiver_thread_id;
    pthread_create(&receiver_thread_id, NULL, (void *)receiverThread, (void *)rcvr);

    pthread_t sender_thread_id;
    pthread_create(&sender_thread_id, NULL, (void *)senderThread, (void *)sndr);


    pthread_join(receiver_thread_id, NULL);
    pthread_join(sender_thread_id, NULL);
}

int startShuffle(int connfd, hash_map *map, int total_shuffle_size) {
    //printMap(map);
    hash_map_iterator *itr = createIterator(map);

    int per_chunck_record = 2;
    for(int i = 0; i < total_shuffle_size; i++) {
        /* Receive chunck_fetch_request */
        int ret = receiveChunckFetchRequest(connfd);
        if(ret != 0) {
            printf("ERROR: receiving chunck_fetch_request()");
        }

        //const char *messages[] = {"hello", "beautiful", "world"};
        int record_count = 0;
        char **chunck = getChunk(itr, per_chunck_record, &record_count);
        //sleep(1);
        if(chunck == NULL) {
            printf("INFO: No more data to push out!\n");
            // Send end of record send??
            break;
        }
        /* Send chunk_fetch_reply */
        sendChunckFetchReply(connfd, chunck, record_count);

        for(int k = 0; k < per_chunck_record * 2; k++) {
            //printf("%d : %s\n", k, chunck[k]);
            free(chunck[k]);
        }
        free(chunck);
    }
    return 0;
}

int main(int argc, char *argv[]) 
{
    printf("INFO: Staring Mapper\n");
    char *IPAddress = (char *) malloc(sizeof(char) * IP_ADDR_MAX_LEN);
    strcpy(IPAddress, argv[1]);
    int port = atoi(argv[2]);
    printf("INFO: Addr -> %s @ %d\n", IPAddress, port);

    hash_map *map = generateData();

    /* 1. Establish Connection */
    int sockfd, connfd; 
    establishConnection(IPAddress, port, &sockfd, &connfd);

    /* 2. Initiate Shuffle Init */
    int total_shuffle_size = (int) malloc(sizeof(int)); 
    int ret = receiveOpenMessage(connfd, &total_shuffle_size); 
    if(ret != 0) {
        printf("ERROR: Open Message deserialization failed!\n");
        sendOpenMessageAck(connfd, 1);
        exit(0);
    }
    printf("Total Shuffle size: %d\n", total_shuffle_size);
    sendOpenMessageAck(connfd, 0);

    //ret = startShuffle(connfd, map, total_shuffle_size);
    startShuffleThreads(connfd, map, total_shuffle_size);
  
    // After chatting close the socket 
    closeConnection(sockfd);

    printf("INFO: Closing Mapper\n");
    return 0; 
} 
