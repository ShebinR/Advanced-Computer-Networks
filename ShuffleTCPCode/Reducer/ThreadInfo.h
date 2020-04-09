#include "Constants.h"
#ifndef THREADINFO_H
#define THREADINFO_H

typedef struct thread_info {
    char IPAddress[IP_ADDR_MAX_LEN];
    int port;
}thread_info;

#endif
