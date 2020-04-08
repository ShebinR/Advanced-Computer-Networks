#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
 
#define MAX 80 

#include "Constants.h"
#include "Protocol.h"
  
// Function designed for chat between client and server. 
void func(int sockfd) 
{ 
    char buff[MAX]; 
    int n; 
    // infinite loop for chat 
    for (;;) { 
        bzero(buff, MAX); 
  
        // read the message from client and copy it in buffer 
        read(sockfd, buff, sizeof(buff)); 
        // print buffer which contains the client contents 
        printf("From client: %s\t To client : ", buff); 
        bzero(buff, MAX); 
        n = 0; 
        // copy server message in the buffer 
        while ((buff[n++] = getchar()) != '\n') 
            ; 
  
        // and send that buffer to client 
        write(sockfd, buff, sizeof(buff)); 
  
        // if msg contains "Exit" then server exit and chat ended. 
        if (strncmp("exit", buff, 4) == 0) { 
            printf("Server Exit...\n"); 
            break; 
        } 
    } 
}

void getOpenMessage(int sockfd) {
    OpenMessage *msg = (OpenMessage *) malloc(sizeof(OpenMessage));
    read(sockfd, msg, sizeof(msg));

    printf("Blocks Per response : %d\n", msg->blocksPerResponse);
} 
  
int main(int argc, char *argv[]) 
{
    printf("INFO: Staring Mapper\n");
    char *IPAddress = (char *) malloc(sizeof(char) * IP_ADDR_MAX_LEN);
    strcpy(IPAddress, argv[1]);
    int port = atoi(argv[2]);
    printf("INFO: Addr -> %s @ %d\n", IPAddress, port);
    
    int sockfd, connfd; 
    struct sockaddr_in servaddr, cli; 
  
    // socket create and verification 
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) { 
        printf("socket creation failed...\n"); 
        exit(0); 
    }
 
    bzero(&servaddr, sizeof(servaddr)); 
    // assign IP, PORT 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = inet_addr(IPAddress); 
    servaddr.sin_port = htons(port); 
  
    // Binding newly created socket to given IP and verification 
    if ((bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) != 0) { 
        printf("ERROR: socket bind failed...\n"); 
        exit(0); 
    } 
    else
        printf("INFO: Socket successfully binded..\n"); 
  
    // Now server is ready to listen and verification 
    if ((listen(sockfd, 5)) != 0) { 
        printf("ERROR: Listen failed...\n"); 
        exit(0); 
    } 
    else
        printf("INFO: Server listening..\n");
 
    socklen_t len = sizeof(cli); 
    // Accept the data packet from client and verification 
    connfd = accept(sockfd, (struct sockaddr *)&cli, &len); 
    if (connfd < 0) { 
        printf("ERROR: server acccept failed...\n"); 
        exit(0); 
    } 
    else
        printf("INFO: Server acccept the client...\n"); 
  
    // Function for chatting between client and server 
    //func(connfd);

    getOpenMessage(sockfd); 
  
    // After chatting close the socket 
    close(sockfd); 
} 
