#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>
#include <arpa/inet.h> 
#include <sys/socket.h>
#include <unistd.h>
 
#define MAX 80

#include "ThreadInput.h"
#include "Protocol.h"
 
void func(int sockfd) 
{ 
    char buff[MAX]; 
    int n; 
    for (;;) { 
        bzero(buff, sizeof(buff)); 
        printf("Enter the string : "); 
        n = 0; 
        while ((buff[n++] = getchar()) != '\n') 
            ; 
        write(sockfd, buff, sizeof(buff)); 
        bzero(buff, sizeof(buff)); 
        read(sockfd, buff, sizeof(buff)); 
        printf("From Server : %s", buff); 
        if ((strncmp(buff, "exit", 4)) == 0) { 
            printf("Client Exit...\n"); 
            break; 
        } 
    } 
}

void sendOpenMessage(int sockfd) {
    OpenMessage *msg = (OpenMessage *) malloc(sizeof(OpenMessage));
    msg->blocksPerResponse = 10;

    write(sockfd, msg, sizeof(msg));

    return;
}
  
void connectToServer(void *input) 
{
    //printf("IP Address : %s\n", ((struct ThreadInput *)input)->IPAddress);
    //printf("Port : %d\n", ((struct ThreadInput *)input)->port);

    char *IPAddress = ((struct ThreadInput *)input)->IPAddress;
    int port = ((struct ThreadInput *)input)->port;
    int sockfd, connfd; 
    struct sockaddr_in servaddr, cli; 
  
    // socket create and varification 
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) { 
        printf("ERROR: Socket creation failed...\n"); 
        return; 
    }

    // assign IP, PORT 
    bzero(&servaddr, sizeof(servaddr)); 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = inet_addr(IPAddress); 
    servaddr.sin_port = htons(port); 
  
    // connect the client socket to server socket
    printf("INFO: Initiating a TCP connection with %s @ %d\n", IPAddress, port); 
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) { 
        printf("ERROR: connection with the server failed...\n"); 
        return; 
    }
    printf("INFO: Connection success! Starting transfer!"); 
  
    // function for chat 
    //func(sockfd);

    sendOpenMessage(sockfd); 
  
    // close the socket 
    close(sockfd);

    return;
} 
