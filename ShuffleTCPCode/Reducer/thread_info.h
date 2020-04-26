#include "constants.h"
#include "queue.h"
#include "hash_map_group_by_key.h"

#ifndef THREADINFO_H
#define THREADINFO_H

typedef struct stats_mapper {
    char server_name[MAX_SERVER_NAME];
    int N_CF_Reqs_sent;
    int N_CF_Reps_rcvd;
    int number_of_request_blocks;
    int number_of_reply_blocks;
    size_t SO_Reqs_sent;
    size_t SO_Reps_rcvd;
    clock_t r_start[MAX_SHUFFLE_SIZE + 10];
    clock_t r_end[MAX_SHUFFLE_SIZE + 10];
    double total_rr_latency;
} stats_mapper;

typedef struct thread_info {
    char server_name[MAX_SERVER_NAME];
    char IPAddress[IP_ADDR_MAX_LEN];
    int port;
    Queue *result_queue;
    int max_reqs_in_flight_per_server;
    int max_record_per_reply;
    int total_shuffle_size;
    stats_mapper *stat_m;
}thread_info;

typedef struct thread_info_grouper {
    char thread_name[MAX_SERVER_NAME];
    Queue *result_queue;
    hash_map_group_by_key *map;
    int *mapper_status;
} thread_info_grouper;

#endif
