#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>
#include <strings.h>
#include <arpa/inet.h> 
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/tcp.h>
 
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
    	sendChunckFetchRequest(sockfd, step);
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
        int max_reqs_in_flight_per_server, int total_shuffle_size, int reply_size) {
    int total_steps = total_shuffle_size / max_reqs_in_flight_per_server;

    printf("FROM : %s\n", server_name);
    for(int step = 0; step < total_steps; step++) {

        for(int i = 0; i < max_reqs_in_flight_per_server; i++) {
            printf("COMMUNICATION THREAD: Sending chunck fetch request()\n");
            fflush(stdout);
            /* Send chuch_fetch_request */
            sendChunckFetchRequest(sockfd, i);
            printf("COMMUNICATION THREAD: Sending chunck fetch request() success!\n");
            fflush(stdout);
        }

        for(int i = 0; i < max_reqs_in_flight_per_server; i++) {
            /* Receive chuch_fetch_reply */
            uint8_t *buf = (uint8_t *) malloc (sizeof(uint8_t) * MAX_MSG_SIZE);
            size_t *len = (size_t *) malloc (sizeof(size_t));
            printf("COMMUNICATION THREAD: Waiting for chunk fetch reply()\n");
            receiveChunckFetchReply(sockfd, buf, len, reply_size);
            printf("COMMUNICATION THREAD: Received chunk fetch reply()\n");
            fflush(stdout);
            int ret = enQueue(result_queue, buf, *len);
            if(ret != 0) 
                printf("COMMUNICATION THREAD: Enqueue failed!\n");
            fflush(stdout);
        }
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

    int sockfd, connfd, ret;
 
    printf("INFO: Thread ID:: %d Contacting : %s\n", (int)t, server_name);
 
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
        max_reqs_in_flight_per_server, total_shuffle_size, reply_size);
  
  
    /* N. Close the socket */
    close(sockfd);

    return;
} 
