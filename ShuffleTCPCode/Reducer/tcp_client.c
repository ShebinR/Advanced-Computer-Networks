#define _GNU_SOURCE
#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>
#include <strings.h>
#include <arpa/inet.h> 
#include <sys/socket.h>
#include <unistd.h>
#include <sched.h>
#include <pthread.h>
#include <netinet/tcp.h>
#include <time.h>
#include <sys/time.h> 
#include <x86intrin.h> 
 
#define MAX 80

#include "thread_info.h"
#include "message_formats.pb-c.h"
#include "communication.h"
#include "constants.h"
 
int establishConnection(char *IPAddress, int port, int *sockfd) {
    struct sockaddr_in servaddr; 
    // socket create and varification 
    *sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (*sockfd == -1) { 
        printf("ERROR: Socket creation failed...\n"); 
        return -1; 
    }

    int flag = 1;
    if(setsockopt(*sockfd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag))) { 
        printf("ERROR: Socket opt setting\n");
        return -1;
    } 
    // assign IP, PORT 
    bzero(&servaddr, sizeof(servaddr)); 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = inet_addr(IPAddress); 
    servaddr.sin_port = htons(port); 
  
    // connect the client socket to server socket
    //printf("INFO: Initiating a TCP connection with %s @ %d\n", IPAddress, port); 
    if (connect(*sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) { 
        printf("ERROR: connection with the server failed...\n"); 
        return -1; 
    }
    return 0;
}
int startShuffleSingleRequest(int sockfd, Queue *result_queue, char *server_name,
        int max_reqs_in_flight_per_server, int total_shuffle_size, int reply_size) {
    int total_steps = total_shuffle_size;

    for(int step = 0; step < total_steps; step++) {
    	printf("FROM : %s\n", server_name);
        /* Send chuch_fetch_request */
    	//sendChunckFetchRequest(sockfd, step);
   	/* Receive chuch_fetch_reply */
    	uint8_t *buf = (uint8_t *) malloc (sizeof(uint8_t) * MAX_MSG_SIZE);
    	size_t *len = (size_t *) malloc (sizeof(size_t));
    	receiveChunckFetchReply(sockfd, buf, len, reply_size);
    	int ret = enQueue(result_queue, buf, *len);
    	if(ret == 0)
	        printf("INFO: Enqueued received data!\n");
    }

    return 0;
}

int startShuffle(int sockfd, Queue *result_queue, char *server_name,
        int max_reqs_in_flight_per_server, int total_shuffle_size, int reply_size, stats_mapper *stat_m) {
    int total_steps = total_shuffle_size / max_reqs_in_flight_per_server;

    printf("FROM : %s\n", server_name);
    for(int step = 0; step < total_steps; step++) {

        struct timeval start_t[max_reqs_in_flight_per_server], end_t[max_reqs_in_flight_per_server];
        unsigned int start[max_reqs_in_flight_per_server];
        for(int k = 0; k < max_reqs_in_flight_per_server; k++)
            start[k] = 0;
        uint64_t time1[max_reqs_in_flight_per_server], time2[max_reqs_in_flight_per_server];
        
        for(int i = 0; i < max_reqs_in_flight_per_server; i++) {
            /* Send chuch_fetch_request */
            // i % 127 : Not a good solution though. Be careful about the multiple write/read situation between reader writter.
            // To solve this, the reader always reads 2 bytes of request. Thus, 128 makes its fail in certain situations due to race-condition.
            size_t tw_bytes, w_bytes;

            time1[i] = __rdtscp(&start[i]);
            //stat_m->r_start[stat_m->N_CF_Reqs_sent] = clock();
            gettimeofday(&start_t[i], NULL);

            sendChunckFetchRequest(sockfd, (i % 127), &tw_bytes, &w_bytes);
            printf("CT: %s CF Req %d TOW = %zu bytes W = %zu bytes\n",
                        server_name, stat_m->N_CF_Reqs_sent, tw_bytes, w_bytes);
            fflush(stdout);
            stat_m->N_CF_Reqs_sent++;
            stat_m->SO_Reqs_sent += w_bytes;
        }
        stat_m->number_of_request_blocks++;

        for(int i = 0; i < max_reqs_in_flight_per_server; i++) {
            /* Receive chuch_fetch_reply */
            uint8_t *buf = (uint8_t *) malloc (sizeof(uint8_t) * MAX_MSG_SIZE);
            size_t tr_bytes = reply_size, r_bytes;

            receiveChunckFetchReply(sockfd, buf, &r_bytes, tr_bytes);
            //CF Reply 41 TOR = 10880 bytes R = 10880 bytes
            printf("CT: %s CF Reply %d TOR = %zu bytes R = %zu bytes\n",
                server_name, stat_m->N_CF_Reps_rcvd, tr_bytes, r_bytes);
            fflush(stdout);
            int ret = enQueue(result_queue, buf, r_bytes);
            if(ret != 0) {
                printf("CT: Enqueue failed!\n");
                fflush(stdout);
            }
            gettimeofday(&end_t[i], NULL);
            time2[i] = __rdtscp(&start[i]) - time1[i];
            //stat_m->r_end[stat_m->N_CF_Reps_rcvd] = clock();

            double time_taken;
            time_taken = (end_t[i].tv_sec - start_t[i].tv_sec) * 1e6; 
            time_taken = (time_taken + (end_t[i].tv_usec -  
                              start_t[i].tv_usec)) * 1e-6;
            stat_m->per_tt[stat_m->N_CF_Reps_rcvd] = time_taken * 1000;
            //stat_m->r_diff[stat_m->N_CF_Reps_rcvd] = stat_m->r_end[stat_m->N_CF_Reps_rcvd] - stat_m->r_start[stat_m->N_CF_Reps_rcvd];
            stat_m->rdts_cc[stat_m->N_CF_Reps_rcvd] = time2[i];
            //printf("Clock reqs : %ld \n", stat_m->r_end[stat_m->N_CF_Reps_rcvd]);
            //fflush(stdout);
            stat_m->N_CF_Reps_rcvd++;
            stat_m->SO_Reps_rcvd += r_bytes;
        }
        stat_m->number_of_reply_blocks++;
    }

    return 0;
}
  
void connectToServer(void *input) 
{
    //printf("IP Address : %s\n", ((thread_info *)input)->IPAddress);
    //printf("Port : %d\n", ((thread_info *)input)->port);
    pthread_t t = pthread_self();

    char *server_name = ((thread_info *)input)->server_name;
    char *IPAddress = ((thread_info *)input)->IPAddress;
    int port = ((thread_info *)input)->port;
    Queue *result_queue = ((thread_info *)input)->result_queue;
    int max_reqs_in_flight_per_server = ((thread_info *)input)->max_reqs_in_flight_per_server;
    int max_record_per_reply = ((thread_info *)input)->max_record_per_reply;
    int total_shuffle_size = ((thread_info *)input)->total_shuffle_size;
    stats_mapper *stat_m = ((thread_info *)input)->stat_m;
    int core_id = ((thread_info *)input)->core_id;

    /* SETTING CPU AFFINITY */
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);
    int set_result = pthread_setaffinity_np(t, sizeof(cpu_set_t), &cpuset);
    if(set_result != 0) {
        printf("ERROR: CPU AFINITY COULD NOT BE SET! exiting ");
        exit(0);
    }

    /* Thread Workload */
    int sockfd, connfd, ret;
 
    printf("INFO: Thread ID:: %d Contacting : %s Core ID: %d\n", (int)t, server_name, core_id);
 
    /* 1. Estabilsh Connection */ 
    ret = establishConnection(IPAddress, port, &sockfd);
    if(ret != 0)
        exit(0); 
    printf("INFO: Connection success! Starting transfer!\n");
  
    /* 2. Initiate Shuffle communication */
    sendOpenMessage(sockfd, total_shuffle_size);
    int reply_size = receiveOpenMessageAck(sockfd);
    if (reply_size == -1) {
        printf("ERROR: Error receiving open_message_ack!");
        exit(0);
    }
    //sleep(2);
    /* 3. Start Shuffle */
    //ret = startShuffleSingleRequest(sockfd, result_queue, server_name,
      //  max_reqs_in_flight_per_server, total_shuffle_size);
    ret = startShuffle(sockfd, result_queue, server_name,
        max_reqs_in_flight_per_server, total_shuffle_size, reply_size, stat_m);
  
  
    /* N. Close the socket */
    close(sockfd);

    return;
} 
