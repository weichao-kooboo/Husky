#ifndef HKY_CYCLE_H_INCLUDED
#define HKY_CYCLE_H_INCLUDED

#include "hky_config.h"
#include "hky_core.h"

struct hky_cycle_s{
    hky_pool_t *pool;

    hky_log_t *log;
    hky_log_t new_log;

    hky_uint_t log_use_stderr;

    hky_list_t open_files;
    hky_list_t shared_memory;

    hky_str_t conf_file;
    hky_str_t conf_param;
    hky_str_t conf_prefix;
    hky_str_t prefix;
};

extern volatile hky_cycle_t *hky_cycle;
extern hky_uint_t             hky_test_config;
extern hky_uint_t             hky_dump_config;
extern hky_uint_t             hky_quiet_mode;

#endif // HKY_CYCLE_H_INCLUDED
