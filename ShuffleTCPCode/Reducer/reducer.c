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

pthread_t contactMapper(char *IPAddress, int port, Queue *result_queue, char *server_name,
        int max_reqs_in_flight_per_server, int max_record_per_reply, int total_shuffle_size, stats_mapper *stat_m) {
    thread_info *node_input = (thread_info *) malloc(sizeof(thread_info));

    strcpy(node_input->server_name, server_name);
    strcpy(node_input->IPAddress, IPAddress);
    node_input->port = port;
    node_input->result_queue = result_queue;
    node_input->max_reqs_in_flight_per_server = max_reqs_in_flight_per_server;
    node_input->max_record_per_reply = max_record_per_reply;
    node_input->total_shuffle_size = total_shuffle_size;
    node_input->stat_m = stat_m;

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

double timeTaken(clock_t start, clock_t end) {
    return ((double) (end - start) / CLOCKS_PER_SEC);
}

void initStats(stats_mapper *stats_m) {
    stats_m->number_of_chuck_fetch_requests = 0;
    stats_m->number_of_request_blocks = 0;
    stats_m->number_of_chuck_fetch_replies_sent = 0;
    stats_m->number_of_reply_blocks = 0;
}

void printStatsMapper(stats_mapper *stats_m) {
    printf("Server Name : %s\n", stats_m->server_name);

    printf("Number of Chunck Fetch Requests sent : %d\n", stats_m->number_of_chuck_fetch_requests);
    printf("Number of Request Blocks sent : %d\n", stats_m->number_of_request_blocks);
    printf("Number of Chunck Fetch Repiles rcvd : %d\n", stats_m->number_of_chuck_fetch_replies_sent);
    printf("Number of Reply Blocks rcvd : %d\n", stats_m->number_of_reply_blocks);
}

int main(int argc, char *argv[]) {
    if(argc != 3) {
        printf("Usage : reducer <config_file_name> <number_of_mappers>\n");
        exit(0);
    }
    clock_t app_start = clock(); 
    printf("INFO: Starting Reducer..\n");
    char fileName[LINE_LENGTH];
    strcpy(fileName, argv[1]);
    printf("INFO: Conf. file Name :: %s\n", fileName);
    int number_of_servers = atoi(argv[2]);
    printf("Number of mappers : %d\n", number_of_servers);

    /* Mapper Data Configs */
    int max_reqs_in_flight_per_server = MAX_REQ_IN_FLIGHT / number_of_servers;
    int max_record_per_reply = MAX_REPLY_SIZE;
    int total_shuffle_size = MAX_SHUFFLE_SIZE;

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
    stats_mapper *stats_m[number_of_servers];
    for(int i = 0; i < number_of_servers; i++) {
        stats_m[i] = (stats_mapper *) malloc(sizeof(stats_mapper));
        initStats(stats_m[i]);
    }
    for(int i  = 0; i < number_of_servers; i++) {
        char server_name[MAX_SERVER_NAME];
        snprintf(server_name, MAX_SERVER_NAME, "server_%d", i);
        strcpy(stats_m[i]->server_name, server_name);
        threads[i] = contactMapper(IPAddress[i], ports[i], result_queue, server_name,
                max_reqs_in_flight_per_server, max_record_per_reply, total_shuffle_size, stats_m[i]);
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

    clock_t app_end = clock(); 
    printf("\n");
    printf("INFO: In the main thread..\n");
    printf("CONTENTS OF THE QUEUE\n");
    printf("=====================================\n");
    printQueue(result_queue);
    printf("=====================================\n");
    printf("\n");
    printf("INFO: Group by key function \n");
    printf("=====================================\n");
    //printMap(result_map);
    printf("=====================================\n");
    
    printf("Reducer done!\n\n");
    printf("------------------------------------------------\n");
    printf("\t\t\t STATS \n");
    printf("------------------------------------------------\n");
    printf("Reducer Time Taken to Complete : %f\n", timeTaken(app_start, app_end));
    for(int i = 0; i < number_of_servers; i++) {
        printStatsMapper(stats_m[i]);
    }
    printf("------------------------------------------------\n");
    return 0; 
}
