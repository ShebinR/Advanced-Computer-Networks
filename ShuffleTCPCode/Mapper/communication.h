#ifndef COMMUNICATION_H
#define COMMUNICATION_H

void sendOpenMessage(int sockfd, int shuffle_size);
int receiveOpenMessage(int sockfd, int *total_shuffle_size);

void sendOpenMessageAck(int sockfd, int success);
int receiveOpenMessageAck(int sockfd);

void sendChunckFetchRequest(int sockfd);
int receiveChunckFetchRequest(int sockfd);

int receiveChunckFetchReply(int sockfd, uint8_t *buf, size_t *len);
void sendChunckFetchReply(int sockfd, char **messages, int num_of_records);

char** deserializeChunkFetchReply(uint8_t *buf, size_t len, int *no_of_records);
#endif
