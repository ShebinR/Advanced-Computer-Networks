#ifndef THREAD_INFO_MAPPER_H
#define THREAD_INFO_MAPPER_H

typedef struct sender_thread_info {
    pthread_mutex_t *request_update_lock;
    int *pending_request;
    int connfd;
    int per_chunck_record;
    hash_map_iterator *itr;
    int *last_request;
    int total_shuffle_size;
} sender_thread_info;

typedef struct receiver_thread_info {
    pthread_mutex_t *request_update_lock;
    int *pending_request;
    int connfd;
    int *last_request;
    int total_shuffle_size;
} receiver_thread_info;

#endif
