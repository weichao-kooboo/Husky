#ifndef HKY_INET_H_INCLUDED
#define HKY_INET_H_INCLUDED

#include "hky_config.h"
#include "hky_core.h"

typedef struct {
        struct sockaddr *sockaddr;
        socklen_t socklen;
        hky_str_t name;
}   hky_addr_t;

#endif // HKY_INET_H_INCLUDED
