#ifndef COMMUNICATION_H
#define COMMUNICATION_H

void sendOpenMessage(int sockfd, int block_size);
int receiveOpenMessage(int sockfd);

void sendOpenMessageAck(int sockfd, int success);
int receiveOpenMessageAck(int sockfd);

void sendChunckFetchRequest(int sockfd);
int receiveChunckFetchRequest(int sockfd);

int receiveChunckFetchReply(int sockfd);
void sendChunckFetchReply(int sockfd);

#endif
