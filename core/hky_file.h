#ifndef HKY_FILE_H_INCLUDED
#define HKY_FILE_H_INCLUDED

#include "hky_config.h"
#include "hky_core.h"

struct hky_file_s{
    hky_fd_t fd;
    hky_str_t name;
};

hky_int_t hky_get_full_name(hky_pool_t *pool,hky_str_t *prefix,hky_str_t *name);

#endif // HKY_FILE_H_INCLUDED
