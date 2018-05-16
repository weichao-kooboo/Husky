#ifndef HKY_ARRAY_H_INCLUDED
#define HKY_ARRAY_H_INCLUDED

#include "hky_config.h"
#include "hky_core.h"

typedef struct {
    void *elts;
    hky_uint_t nelts;
    size_t size;
    hky_uint_t nalloc;
    hky_pool_t *pool;
}   hky_array_t;

#endif // HKY_ARRAY_H_INCLUDED
