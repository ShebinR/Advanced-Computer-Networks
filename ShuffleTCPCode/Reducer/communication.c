#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "message_formats.pb-c.h"
#include "constants.h"

/* ------------------------------- RECEIVING FUNCTIONS ------------------------- */
int receiveOpenMessage(int sockfd) {
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

int receiveChunckFetchReply(int sockfd) {
    ChunckFetchReply *msg;
    uint8_t buf[MAX_MSG_SIZE];
    unsigned i;
   
    size_t msg_len = read(sockfd, buf, MAX_MSG_SIZE);
    msg = chunck_fetch_reply__unpack (NULL, msg_len, buf); // Deserialize the serialized input
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

    chunck_fetch_reply__free_unpacked(msg,NULL); // Free the message from unpack()
    return 0;
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

void createChunckFetchRequest(void **buf, unsigned int *len) {
    ChunckFetchRequest msg = CHUNCK_FETCH_REQUEST__INIT; // AMessage
    
    msg.chunck_size = 1;
    *len = chunck_fetch_request__get_packed_size(&msg);
    *buf = malloc(*len);
    chunck_fetch_request__pack(&msg, *buf);

    //printSerializedMessage(buf, len); 
}

void sendChunckFetchRequest(int sockfd) {
    void *buf;                     // Buffer to store serialized data
    unsigned len;                  // Length of serialized data

    printf("SENDING: chunck_fetch_request{}!\n");
    createChunckFetchRequest(&buf, &len);
    write(sockfd, buf, len);

    free(buf);                      // Free the allocated serialized buffer
}

void createChunckFetchReply(char **messages, void **buf, unsigned int *len) {
    ChunckFetchReply msg = CHUNCK_FETCH_REPLY__INIT;  // Message (repeated string)
    unsigned size=0;                                  // Length of serialized data

    msg.n_record_info = 10;                           // Save number of repeated strings
    for(int i = 0; i < 10; i++) {                     // Find amount of memory to allocate
        size += ((int)strlen(messages[i]) + 2);
    }

    msg.record_info = malloc (sizeof (char) * size);  // Allocate memory to store string
    for(int j = 0; j < msg.n_record_info; j++) {
        msg.record_info[j] = (char*)messages[j];      // Access msg.c[] as array
    }

    *len = chunck_fetch_reply__get_packed_size (&msg);  // This is calculated packing length
    *buf = malloc (*len);                               // Allocate required serialized buffer length
    chunck_fetch_reply__pack (&msg, *buf);              // Pack the data

    free (msg.record_info);                             // Free storage for repeated string
}

void sendChunckFetchReply(int sockfd, char **messages) {
    void *buf;                                          // Buffer to store serialized data
    unsigned len;
    
    createChunckFetchReply(messages, &buf, &len);
    write(sockfd, buf, len);

    free (buf);                                         // Free serialized buffer
}

void printSerializedMessage(void *buf, int len) {
    printf("DEBUG: writing %d serialized bytes\n", len); // See the length of message

    for(int i = 0; i < len; i++)
        printf("%d ", (int)(buf + sizeof(int) * i));
    printf("\n");
}

/* ------------------------------- SENDING FUNCTIONS ------------------------- */
