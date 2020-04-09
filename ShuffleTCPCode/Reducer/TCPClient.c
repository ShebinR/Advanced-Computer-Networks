#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>
#include <arpa/inet.h> 
#include <sys/socket.h>
#include <unistd.h>
 
#define MAX 80

#include "ThreadInfo.h"
#include "message_formats.pb-c.h"
 
void printSerializedMessage(void *buf, int len) {
    printf("DEBUG: writing %d serialized bytes\n", len); // See the length of message

    for(int i = 0; i < len; i++)
        printf("%d ", (int)(buf + sizeof(int) * i));
    printf("\n");
}

void createOpenMessage(int block_size, void **buf, unsigned int *len) {
    OpenMessage msg = OPEN_MESSAGE__INIT; // AMessage
    
    msg.block_size = block_size;
    *len = open_message__get_packed_size(&msg);
    *buf = malloc(*len);
    open_message__pack(&msg, *buf);

    //printSerializedMessage(buf, len); 
}

void sendOpenMessage(int sockfd) {
    void *buf;                     // Buffer to store serialized data
    unsigned len;                  // Length of serialized data

    printf("INFO: Sending open_message to the mapper!\n");
    createOpenMessage(23456, &buf, &len);
    write(sockfd, buf, len);

    free(buf);                      // Free the allocated serialized buffer
}

void establishConnection(char *IPAddress, int port, int *sockfd) {
    struct sockaddr_in servaddr; 
    // socket create and varification 
    *sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (*sockfd == -1) { 
        printf("ERROR: Socket creation failed...\n"); 
        return; 
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
        return; 
    }
}
  
void connectToServer(void *input) 
{
    //printf("IP Address : %s\n", ((thread_info *)input)->IPAddress);
    //printf("Port : %d\n", ((thread_info *)input)->port);
    char *IPAddress = ((thread_info *)input)->IPAddress;
    int port = ((thread_info *)input)->port;
    int sockfd, connfd; 
 
    /* 1. Estabilsh Connection */ 
    establishConnection(IPAddress, port, &sockfd); 
    printf("INFO: Connection success! Starting transfer!\n");
  
    /* 2. Send openMessage */
    sendOpenMessage(sockfd); 
  
    /* N. Close the socket */
    close(sockfd);

    return;
} 
