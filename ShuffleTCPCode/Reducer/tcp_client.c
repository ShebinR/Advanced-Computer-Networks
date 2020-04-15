#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>
#include <arpa/inet.h> 
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
 
#define MAX 80

#include "thread_info.h"
#include "message_formats.pb-c.h"
#include "communication.h"
 
int establishConnection(char *IPAddress, int port, int *sockfd) {
    struct sockaddr_in servaddr; 
    // socket create and varification 
    *sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (*sockfd == -1) { 
        printf("ERROR: Socket creation failed...\n"); 
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

int startShuffle(int sockfd, int total_shuffle_size, Queue *result_queue) {
    for(int i = 0; i < total_shuffle_size; i++) {
        /* Send chuch_fetch_request */
        sendChunckFetchRequest(sockfd);

        /* Receive chuch_fetch_reply */
        uint8_t *buf = (uint8_t *) malloc (sizeof(uint8_t) * MAX_MSG_SIZE);
        size_t *len = (size_t *) malloc (sizeof(size_t));
        receiveChunckFetchReply(sockfd, buf, len);
        int ret = enQueue(result_queue, buf, *len);
        if(ret == 0)
            printf("INFO: Enqueued received data!\n");
    }

    return 0;
}
  
void connectToServer(void *input) 
{
    //printf("IP Address : %s\n", ((thread_info *)input)->IPAddress);
    //printf("Port : %d\n", ((thread_info *)input)->port);
    pthread_t t = pthread_self();
    printf("INFO: Thread ID:: %d\n", t);
    char *IPAddress = ((thread_info *)input)->IPAddress;
    int port = ((thread_info *)input)->port;
    Queue *result_queue = ((thread_info *)input)->result_queue;
    int sockfd, connfd, ret; 
 
    /* 1. Estabilsh Connection */ 
    ret = establishConnection(IPAddress, port, &sockfd);
    if(ret != 0)
        exit(0); 
    printf("INFO: Connection success! Starting transfer!\n");
  
    /* 2. Initiate Shuffle communication */
    int total_shuffle_size = 100;
    sendOpenMessage(sockfd, total_shuffle_size);
    ret = receiveOpenMessageAck(sockfd);
    if (ret != 0) {
        printf("ERROR: Error receiving open_message_ack!");
        exit(0);
    }

    /* 3. Start Shuffle */
    ret = startShuffle(sockfd, total_shuffle_size, result_queue);
    printf("CONTENTS OF THE QUEUE\n");
    printf("=====================================\n");
    printQueue(result_queue);
    printf("=====================================\n");

  
    /* N. Close the socket */
    close(sockfd);

    return;
} 
