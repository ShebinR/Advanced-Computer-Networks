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

int startShuffle(int sockfd) {
    /* Send chuch_fetch_request */
    sendChunckFetchRequest(sockfd);

    /* Receive chuch_fetch_reply */
    receiveChunckFetchReply(sockfd);

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
    int sockfd, connfd, ret; 
 
    /* 1. Estabilsh Connection */ 
    ret = establishConnection(IPAddress, port, &sockfd);
    if(ret != 0)
        exit(0); 
    printf("INFO: Connection success! Starting transfer!\n");
  
    /* 2. Initiate Shuffle communication */
    sendOpenMessage(sockfd, 1500);
    ret = receiveOpenMessageAck(sockfd);
    if (ret != 0) {
        printf("ERROR: Error receiving open_message_ack!");
        exit(0);
    }

    /* 3. Start Shuffle */
    ret = startShuffle(sockfd);
  
    /* N. Close the socket */
    close(sockfd);

    return;
} 
