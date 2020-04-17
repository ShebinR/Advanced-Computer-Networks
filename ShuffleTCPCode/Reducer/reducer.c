#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include "tcp_client.h"
#include "constants.h"
#include "thread_info.h"
#include "hash_map_group_by_key.h"
#include "group_by_key_reducer.h"

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

pthread_t contactMapper(char *IPAddress, int port, Queue *result_queue, char *server_name) {
    thread_info *node_input = (thread_info *) malloc(sizeof(thread_info));
    strcpy(node_input->server_name, server_name);
    strcpy(node_input->IPAddress, IPAddress);
    node_input->port = port;
    node_input->result_queue = result_queue;
    printf("INFO: contacting Mapper @ %s:%d\n", node_input->IPAddress, node_input->port);

    /* Creating a thread */
    pthread_t tid;
    pthread_create(&tid, NULL, (void *)connectToServer, (void *)node_input);

    return tid;
}

pthread_t createGroupByReducerThread(Queue *queue, hash_map_group_by_key *map, char *thread_name, int *mapper_status) {
    thread_info_grouper *grouper_info = (thread_info_grouper *) malloc(sizeof(thread_info_grouper));
    strcpy(grouper_info->thread_name, thread_name);
    grouper_info->result_queue = queue;
    grouper_info->map = map;
    grouper_info->mapper_status = mapper_status;

    /* Creating a thread */
    pthread_t tid;
    pthread_create(&tid, NULL, (void *)groupByKeyReducer, (void *)grouper_info);

    return tid;
}

int main(int argc, char *argv[]) {
    if(argc != 3) {
        printf("Usage : reducer <config_file_name> <number_of_mappers>\n");
        exit(0);
    } 
    printf("INFO: Starting Reducer..\n");
    char fileName[LINE_LENGTH];
    strcpy(fileName, argv[1]);
    printf("INFO: Conf. file Name :: %s\n", fileName);
    int number_of_servers = atoi(argv[2]);
    printf("Number of mappers : %d\n", number_of_servers);

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

    /* Create a Queue */
    Queue *result_queue = createQueue(MAX_QUEUE_CAPACITY);
    /* Create a reult HashMap */
    hash_map_group_by_key *result_map = createHashMapGroupByKey(HASH_MAP_SIZE);
  
    /* Connect to Server in Thread */
    //connectToServer(IPAddress[0], ports[0]);
    pthread_t threads[number_of_servers];
    for(int i  = 0; i < number_of_servers; i++) {
        char server_name[MAX_SERVER_NAME];
        snprintf(server_name, MAX_SERVER_NAME, "server_%d", i);
        threads[i] = contactMapper(IPAddress[i], ports[i], result_queue, server_name);
    }
    //pthread_t tid0 = contactMapper(IPAddress[0], ports[0], result_queue, "server_0");
    //pthread_t tid1 = contactMapper(IPAddress[1], ports[1], result_queue, "server_1");

    /* Create a reducer Thread */
    int mapper_status = 0;
    pthread_t tid_r = createGroupByReducerThread(result_queue, result_map, "group_by_key", &mapper_status);

    /* Waiting for theads to complete */
    //pthread_join(tid0, NULL);
    //pthread_join(tid1, NULL);
    for(int i = 0; i < number_of_servers; i++) {
        pthread_join(threads[i], NULL);
    }
    mapper_status = 1;
    pthread_join(tid_r, NULL);

    printf("\n");
    printf("INFO: In the main thread..\n");
    printf("CONTENTS OF THE QUEUE\n");
    printf("=====================================\n");
    printQueue(result_queue);
    printf("=====================================\n");
    printf("\n");
    printf("INFO: Group by key function \n");
    printf("=====================================\n");
    printMap(result_map);
    printf("=====================================\n");
    
    printf("Reducer done!\n");
    return 0; 
}
