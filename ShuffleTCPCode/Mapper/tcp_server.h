#ifndef TCPSERVER_H
#define TCPSERVER_H

void establishConnection(char *IPAddress, int port, int *sockfd, int *connfd);
void closeConnection(int sockfd);

#endif
