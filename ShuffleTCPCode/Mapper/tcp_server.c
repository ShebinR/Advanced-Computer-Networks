#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <sys/socket.h> 
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <arpa/inet.h>

void establishConnection(char *IPAddress, int port, int *sockfd, int *connfd) {
    struct sockaddr_in servaddr, cli; 
  
    // socket create and verification 
    *sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (*sockfd == -1) { 
        printf("socket creation failed...\n"); 
        exit(0); 
    }

    int enable = 1;
    if (setsockopt(*sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        printf("setsockopt(SO_REUSEADDR) failed");
 
    bzero(&servaddr, sizeof(servaddr)); 
    // assign IP, PORT 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = inet_addr(IPAddress); 
    servaddr.sin_port = htons(port); 
  
    // Binding newly created socket to given IP and verification 
    if ((bind(*sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) != 0) { 
        printf("ERROR: socket bind failed...\n"); 
        exit(0); 
    } 
    printf("INFO: Socket successfully binded..\n"); 
  
    // Now server is ready to listen and verification 
    if ((listen(*sockfd, 5)) != 0) { 
        printf("ERROR: Listen failed...\n"); 
        exit(0); 
    } 
    printf("INFO: Server listening..\n");
 
    socklen_t len = sizeof(cli); 
    // Accept the data packet from client and verification 
    *connfd = accept(*sockfd, (struct sockaddr *)&cli, &len); 
    if (connfd < 0) { 
        printf("ERROR: server acccept failed...\n"); 
        exit(0); 
    } 
    else
      printf("INFO: Server acccept the client...\n"); 
}

void closeConnection(int sockfd) {
    printf("INFO: Closing server connection\n");
    int ret = close(sockfd);
    printf("INFO: Close Status : %d\n", ret);
} 
