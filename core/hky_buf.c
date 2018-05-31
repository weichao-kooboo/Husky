#include "hky_config.h"
#include "hky_core.h"

hky_buf_t *
hky_create_temp_buf(hky_pool_t *pool,size_t size){
    hky_buf_t *b;
    b=hky_calloc_buf(pool);
    if (NULL==b) {
        return NULL;
    }
    
    b->start=hky_palloc(pool, size);
    if (b->start==NULL) {
        return NULL;
    }
    
    b->pos=b->start;
    b->last=b->start;
    b->end=b->last+size;
    b->temporary=1;
    
    return b;
}