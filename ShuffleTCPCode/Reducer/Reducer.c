#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include "TCPClient.h"
#include "Constants.h"
#include "ThreadInfo.h"

int readFile(char fileName[], char **contents) {
    int lineIndex = 0;

    FILE *fp = fopen(fileName, "r");
    char line[LINE_LENGTH];

    while(fgets(line, sizeof(line), fp)) {

        int len = strlen(line);
        line[len - 1] = '\0';
        strcpy(contents[lineIndex], line);
        lineIndex++;
    }

    return lineIndex;
}

void printLines(char **contents, int len) {
    for(int i = 0; i < len; i++)
        printf("%s\n", contents[i]);
}

void extractNodeInfo(char **lines, char **IPAddress, int ports[], int len) {

    for(int k = 0; k < len; k++) {
        int i = 0;
        while(lines[k][i] != '\0') {
            if(lines[k][i] == ' ')
                break;
            IPAddress[k][i] = lines[k][i];
            i++;
        }
        IPAddress[k][i] = '\0';
        char portName[10];
        int j = 0;
        while(lines[k][i] != '\0') {
            portName[j] = lines[k][i];
            i++; j++;
        }
        ports[k] = atoi(portName);
    }
}

void printNodeInfo(char **IPAddress, int ports[], int n) {
    printf("INFO: Mapper Info\n");
    for(int i = 0;i < n; i++)
        printf("\t%s @ %d\n", IPAddress[i], ports[i]);
}

pthread_t contactMapper(char *IPAddress, int port) {
    thread_info *node_input = (thread_info *) malloc(sizeof(thread_info));
    strcpy(node_input->IPAddress, IPAddress);
    node_input->port = port;
    printf("INFO: contacting Mapper @ %s:%d\n", node_input->IPAddress, node_input->port);

    /* Creating a thread */
    pthread_t tid;
    pthread_create(&tid, NULL, (void *)connectToServer, (void *)node_input);

    return tid;
}

int main(int argc, char *argv[]) {
    
    printf("INFO: Starting Reducer..\n");
    char fileName[LINE_LENGTH];
    strcpy(fileName, argv[1]);
    printf("INFO: Conf. file Name :: %s\n", fileName);

    /* Read config file */
    char *lines[MAX_LINES];
    for(int i = 0; i < MAX_LINES; i++)
       lines[i] = (char *) malloc(sizeof(char) * LINE_LENGTH);
    int len = readFile(fileName, (char **)&lines);
    //printLines((char **)lines, len);

    /* Extract Mapper Info */
    char *IPAddress[MAX_LINES];
    for(int i = 0; i < MAX_LINES; i++)
        IPAddress[i] = (char *) malloc(sizeof(char) * IP_ADDR_MAX_LEN);
    int ports[MAX_LINES];
    extractNodeInfo(lines, IPAddress, ports, len);
    //printNodeInfo(IPAddress, ports, len);

    /* Connect to Server in Thread */
    //connectToServer(IPAddress[0], ports[0]);
    pthread_t tid = contactMapper(IPAddress[0], ports[0]);

    /* Waiting for theads to complete */
    pthread_join(tid, NULL);

    printf("INFO: In the main thread.. Reducer done!\n");
    return 0; 
}
