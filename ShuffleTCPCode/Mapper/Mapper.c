#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
 
#include "Constants.h"
#include "message_formats.pb-c.h"
#include "hash_map.h"
  
void getOpenMessage(int sockfd) {
    OpenMessage *msg;
    uint8_t buf[MAX_MSG_SIZE];
    char buffer[256];

    size_t msg_len = read(sockfd, buf, MAX_MSG_SIZE);
    //printf("DEBUG: Len = %d\n", msg_len);
    msg = open_message__unpack(NULL, msg_len, buf);	
    if (msg == NULL) {
        fprintf(stderr, "error unpacking incoming message\n");
        return;
    }
    printf("Received: block_size = %d\n",msg->block_size);  // required field

    // Free the unpacked message
    open_message__free_unpacked(msg, NULL);
}

void establishConnection(char *IPAddress, int port, int *sockfd, int *connfd) {
    struct sockaddr_in servaddr, cli; 
  
    // socket create and verification 
    *sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (*sockfd == -1) { 
        printf("socket creation failed...\n"); 
        exit(0); 
    }
 
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
 
void rand_string(char *str, size_t size)
{
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJK";
    if (size) {
        --size;
        for (size_t n = 0; n < size; n++) {
            int key = rand() % (int) (sizeof charset - 1);
            str[n] = charset[key];
        }
        str[size] = '\0';
    }
}

hash_map * generateData() {
    hash_map *map = createHashMap(HASH_MAP_SIZE);

    for(int i = 0; i < MAX_RECORDS; i++) {
        char *key = (char *) malloc(sizeof(char) * KEY_SIZE);
        rand_string(key, KEY_SIZE);
        char *val = (char *) malloc(sizeof(char) * VAL_SIZE);
        rand_string(val, VAL_SIZE);
        insert(map, key, val);

        free(key); free(val);
    }

    //printMap(map);

    return map;
}

int main(int argc, char *argv[]) 
{
    printf("INFO: Staring Mapper\n");
    char *IPAddress = (char *) malloc(sizeof(char) * IP_ADDR_MAX_LEN);
    strcpy(IPAddress, argv[1]);
    int port = atoi(argv[2]);
    printf("INFO: Addr -> %s @ %d\n", IPAddress, port);

    hash_map *map = generateData();

    int sockfd, connfd; 
    establishConnection(IPAddress, port, &sockfd, &connfd);
 
    //getOpenMessage(sockfd); 
    getOpenMessage(connfd); 
  
    // After chatting close the socket 
    close(sockfd);

    return 0; 
} 
