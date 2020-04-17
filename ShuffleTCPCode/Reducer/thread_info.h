#include "constants.h"
#include "queue.h"
#include "hash_map_group_by_key.h"

#ifndef THREADINFO_H
#define THREADINFO_H

typedef struct thread_info {
    char server_name[MAX_SERVER_NAME];
    char IPAddress[IP_ADDR_MAX_LEN];
    int port;
    Queue *result_queue;
}thread_info;

typedef struct thread_info_grouper {
    char thread_name[MAX_SERVER_NAME];
    Queue *result_queue;
    hash_map_group_by_key *map;
    int *mapper_status;
} thread_info_grouper;

#endif
