#include "Constants.h"
#ifndef THREADINPUT_H
#define THREADINPUT_H

struct ThreadInput {
    char IPAddress[IP_ADDR_MAX_LEN];
    int port;
};

#endif
