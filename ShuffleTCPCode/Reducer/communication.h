#include <stdint.h>

#ifndef COMMUNICATION_H
#define COMMUNICATION_H

void sendOpenMessage(int sockfd, int shuffle_size);
int receiveOpenMessage(int sockfd, int *total_shuffle_size);

void sendOpenMessageAck(int sockfd, int success);
int receiveOpenMessageAck(int sockfd);

void sendChunckFetchRequest(int sockfd, int last_block, size_t *tw_bytes, size_t *w_bytes);
int receiveChunckFetchRequest(int sockfd);

int receiveChunckFetchReply(int sockfd, uint8_t *buf, size_t *len, int reply_size);
void sendChunckFetchReply(int sockfd, char **messages, int num_of_records);

char** deserializeChunkFetchReply(uint8_t *buf, size_t len, int *no_of_records);
unsigned int findChunckFetchReplySize(char **messages, int number_of_records);
#endif
