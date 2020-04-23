#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdint.h>
#include <malloc.h>

#include "message_formats.pb-c.h"
#include "constants.h"

/* ------------------------------- RECEIVING FUNCTIONS ------------------------- */
int receiveOpenMessage(int sockfd, int *total_shuffle_size) {
    OpenMessage *msg;
    uint8_t buf[MAX_MSG_SIZE];

    size_t msg_len = read(sockfd, buf, MAX_MSG_SIZE);
    //printf("DEBUG: Len = %d\n", msg_len);
    msg = open_message__unpack(NULL, msg_len, buf);	
    if (msg == NULL) {
        printf("ERROR: Unpacking incoming message\n");
        return -1;
    }
    printf("RECEIVED: open_message{} block_size = %d\n",msg->shuffle_size);  // required field
    *total_shuffle_size = msg->shuffle_size;

    // Free the unpacked message
    open_message__free_unpacked(msg, NULL);

    return 0;
}

int receiveOpenMessageAck(int sockfd) {
    OpenMessageAck *msg;
    uint8_t buf[MAX_MSG_SIZE];

    size_t msg_len = read(sockfd, buf, MAX_MSG_SIZE);
    //printf("DEBUG: Len = %d\n", msg_len);
    msg = open_message_ack__unpack(NULL, msg_len, buf);	
    if (msg == NULL) {
        printf("ERROR: Unpacking incoming message\n");
        return -1;
    }
    printf("RECEIVED: open_message_ack{} success = %d\n",msg->success);  // required field

    // Free the unpacked message
    open_message_ack__free_unpacked(msg, NULL);

    return 0;
}

int receiveChunckFetchRequest(int sockfd) {
    ChunckFetchRequest *msg;
    uint8_t buf[MAX_MSG_SIZE];

    size_t msg_len = read(sockfd, buf, MAX_MSG_SIZE);
    //printf("DEBUG: Len = %d\n", msg_len);
    msg = chunck_fetch_request__unpack(NULL, msg_len, buf);	
    if (msg == NULL) {
        printf("ERROR: Unpacking incoming message\n");
        return -1;
    }
    printf("RECEIVED: chunck_fetch_request{} chunck_fetch = %d\n",msg->chunck_size);  // required field

    // Free the unpacked message
    chunck_fetch_request__free_unpacked(msg, NULL);

    return 0;
}

int receiveChunckFetchReply(int sockfd, uint8_t *buf, size_t *msg_len) {
    ChunckFetchReply *msg;
    unsigned i;
   
    *msg_len = read(sockfd, buf, MAX_MSG_SIZE);
    /*msg = chunck_fetch_reply__unpack (NULL, *msg_len, buf); // Deserialize the serialized input
    if(msg == NULL) { // Something failed
        fprintf(stderr, "error unpacking incoming message\n");
        return -1;
    }
    printf("RECEIVED: \n");
    for (i = 0; i < msg->n_record_info; i++) { // Iterate through all repeated strings
        if(i > 0)
            printf (", ");
        printf("%s", msg->record_info[i]);
    }
    printf ("\n");

    chunck_fetch_reply__free_unpacked(msg, NULL); // Free the message from unpack()
    */
    return 0;
}

char** deserializeChunkFetchReply(uint8_t *buf, size_t msg_len, int *no_of_record) {
    unsigned i;
    ChunckFetchReply *msg = chunck_fetch_reply__unpack (NULL, msg_len, buf); // Deserialize the serialized input
    if(msg == NULL) { // Something failed
        printf("ERROR: Deserializing the message!\n");
        return NULL;
    }
    *no_of_record = msg->n_record_info;
    char **messages = (char **) malloc (sizeof(char *) * msg->n_record_info);
    for(i = 0; i < msg->n_record_info; i++) {
        int len = strlen(msg->record_info[i]);
        char *message = (char *) malloc (sizeof(char) * (len + 1));
        strcpy(message, msg->record_info[i]);
        messages[i] = message;
    }
    chunck_fetch_reply__free_unpacked(msg, NULL); // Free the message from unpack()
    return messages;
}

/* ------------------------------- RECEIVING FUNCTIONS ------------------------- */

/* ------------------------------- SENDING FUNCTIONS ------------------------- */
void createOpenMessageAck(int success, void **buf, unsigned int *len) {
    OpenMessageAck msg = OPEN_MESSAGE_ACK__INIT; // AMessage
    
    msg.success = success;
    *len = open_message_ack__get_packed_size(&msg);
    *buf = malloc(*len);
    open_message_ack__pack(&msg, *buf);

    //printSerializedMessage(buf, len); 
}

void sendOpenMessageAck(int sockfd, int success) {
    void *buf;                     // Buffer to store serialized data
    unsigned len;                  // Length of serialized data

    printf("SENDING: open_message_ack{}!\n");
    createOpenMessageAck(success, &buf, &len);
    write(sockfd, buf, len);

    free(buf);                      // Free the allocated serialized buffer
}

void createOpenMessage(int shuffle_size, void **buf, unsigned int *len) {
    OpenMessage msg = OPEN_MESSAGE__INIT; // AMessage
    
    msg.shuffle_size = shuffle_size;
    *len = open_message__get_packed_size(&msg);
    *buf = malloc(*len);
    open_message__pack(&msg, *buf);

    //printSerializedMessage(buf, len); 
}

void sendOpenMessage(int sockfd, int block_size) {
    void *buf;                     // Buffer to store serialized data
    unsigned len;                  // Length of serialized data

    printf("SENDING: open_message{}!\n");
    createOpenMessage(block_size, &buf, &len);
    write(sockfd, buf, len);

    free(buf);                      // Free the allocated serialized buffer
}

void createChunckFetchRequest(void **buf, unsigned int *len, int last_block) {
    ChunckFetchRequest msg = CHUNCK_FETCH_REQUEST__INIT; // AMessage
    
    msg.chunck_size = last_block;
    *len = chunck_fetch_request__get_packed_size(&msg);
    *buf = malloc(*len);
    chunck_fetch_request__pack(&msg, *buf);

    //printSerializedMessage(buf, len); 
}

void sendChunckFetchRequest(int sockfd, int last_block) {
    void *buf;                     // Buffer to store serialized data
    unsigned len;                  // Length of serialized data

    printf("SENDING: chunck_fetch_request{}!\n");
    createChunckFetchRequest(&buf, &len, last_block);
    write(sockfd, buf, len);
    //send(sockfd, buf, len, MSG_DONTWAIT);
    //usleep(5000);

    free(buf);                      // Free the allocated serialized buffer
}

void sendChunckFetchReply(int sockfd, char **messages, int number_of_records) {
    void *buf;                                          // Buffer to store serialized data
    size_t len;
    ChunckFetchReply msg = CHUNCK_FETCH_REPLY__INIT;  // Message (repeated string)
    unsigned size = 0, i, j;                                  // Length of serialized data

    //printf("No of recs: %d\n", number_of_records);
    msg.n_record_info = number_of_records;                           // Save number of repeated strings
    /*for(i = 0; i < number_of_records; i++) {                     // Find amount of memory to allocate
    	printf("STRING : %s ", messages[i]);
	printf(" len : %d\n", (int)strlen(messages[i]));
    }*/
    for(i = 0; i < number_of_records; i++) {                     // Find amount of memory to allocate
        size += ((int)strlen(messages[i]) + 4);
	//printf("Index I : %d\n", i);
    }

    msg.record_info = malloc (sizeof (char) * size);  // Allocate memory to store string
    for(j = 0; j < msg.n_record_info; j++) {
	//printf("Index J : %d\n", j);
        msg.record_info[j] = (char*)messages[j];      // Access msg.c[] as array
    }
    len = chunck_fetch_reply__get_packed_size (&msg);  // This is calculated packing length
    //printf("Packing message len : Calc Len : %d\n", len);
    buf = malloc (len);                               // Allocate required serialized buffer length
    //printf("Packing message Mem alloc size : %d\n\n", (int)malloc_usable_size(buf));
    chunck_fetch_reply__pack (&msg, buf);              // Pack the data

    //printf("SENDING: chunck_fetch_reply{}!\n");
    write(sockfd, buf, len);
    //usleep(1000);

    free (msg.record_info);                             // Free storage for repeated string
    free (buf);                                         // Free serialized buffer
}

void printSerializedMessage(void *buf, int len) {
    printf("DEBUG: writing %d serialized bytes\n", len); // See the length of message

    for(int i = 0; i < len; i++)
        printf("%d ", (int)(buf + sizeof(int) * i));
    printf("\n");
}

/* ------------------------------- SENDING FUNCTIONS ------------------------- */
