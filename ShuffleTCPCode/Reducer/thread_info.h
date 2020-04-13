#include "constants.h"
#include "queue.h"

#ifndef THREADINFO_H
#define THREADINFO_H

typedef struct thread_info {
    char IPAddress[IP_ADDR_MAX_LEN];
    int port;
    Queue *result_queue;
}thread_info;

#endif
