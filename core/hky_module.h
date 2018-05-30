#ifndef HKY_MODULE_H_INCLUDED
#define HKY_MODULE_H_INCLUDED

#include "hky_config.h"
#include "hky_core.h"
#include "husky.h"

struct hky_module_s{

};
typedef struct{
    hky_str_t name;
    void *(*create_conf)(hky_cycle_t *cycle);
    char *(*init_conf)(hky_cycle_t *cycle,void *conf);
} hky_core_module_t;

extern hky_uint_t hky_max_module;

#endif // HKY_MODULE_H_INCLUDED
