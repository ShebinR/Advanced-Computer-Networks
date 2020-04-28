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
    clock_t *r_start;
    clock_t *r_end;
    clock_t *r_diff;
    clock_t total_r_clocks;
    double total_rr_latency;
    double *per_tt;

    uint64_t *rdts_cc;
    uint64_t rdts_total_cc;
} stats_mapper;

typedef struct stats_grouper {
    clock_t *d_start;
    clock_t *d_end;
    clock_t *d_diff;
    clock_t total_d_clocks;
    double *per_tt;
    double total_deser_latency;

    uint64_t *rdts_cs;
    uint64_t rdts_total_cs;
} stats_grouper;

typedef struct thread_info {
    char server_name[MAX_SERVER_NAME];
    int core_id;
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
    int core_id;
    Queue *result_queue;
    hash_map_group_by_key *map;
    int *mapper_status;
    stats_grouper *stats_g;
} thread_info_grouper;

#endif
