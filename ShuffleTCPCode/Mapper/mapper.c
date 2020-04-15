#include <stdio.h> 
#include <string.h> 
#include <stdlib.h> 
 
#include "constants.h"
#include "hash_map.h"
#include "tcp_server.h"
#include "communication.h"

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

int startShuffle(int connfd, hash_map *map, int total_shuffle_size) {
    //printMap(map);
    hash_map_iterator *itr = createIterator(map);

    //for(int i = 0; i < 5; i++) {
      //  printf("%s\n", chunk[i]);
    //}

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

    ret = startShuffle(connfd, map, total_shuffle_size);
  
    // After chatting close the socket 
    closeConnection(sockfd);

    printf("INFO: Closing Mapper\n");
    return 0; 
} 
