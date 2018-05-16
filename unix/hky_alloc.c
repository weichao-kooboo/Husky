#include "../core/hky_config.h"
#include "../core/hky_core.h"

hky_uint_t hky_pagesize;
hky_uint_t hky_pagesize_shift;
hky_uint_t hky_cacheline_size;

void *
hky_alloc(size_t size,hky_log_t *log){
    void *p;
    p=malloc(size);
    if(p=NULL){
        hky_log_error(HKY_LOG_EMERG,log,hky_errno,
                                            "malloc(%uz) failed",size);
    }
    hky_log_debug2(HKY_LOG_DEBUG_ALLOC,log,0,"malloc: %p:%uz",p,size);

    return p;
}
void *
hky_calloc(size_t size,hky_log_t *log){
    void *p;
    p=hky_alloc(size,log);
    if (p){
        hky_memzero(p,size);
    }
    return p;
}

#if(HKY_HAVE_POSIX_MEMALIGN)
void *
hky_memalign(size_t alignment,size_t size,hky_log_t *log){
    void *p;
    int err;
    err=posix_memalign(&p,alignment,size);

    if(err){
        hky_log_error(HKY_LOG_EMERG,log,err,
                                            "posix_memalign(%uz,%uz) failed",alignment,size);
        p=NULL;
    }
    hky_log_debug3(HKY_LOG_DEBUG_ALLOC,log,0,
                                                "posix_memalign: %p:%uz @%uz",p,size,alignment);
    return p;
}
#elif (HKY_HAVE_MEMALIGN)
void *
hky_memalign(size_t alignment,size_t size,hky_log_t *log){
    void *p;
    p=memalign(alignment,size);
    if(p==NULL){
        hky_log_error(HKY_LOG_EMERG,log,hky_errno,
                                            "memalign(%uz,%uz) failed",alignment,size);
    }
    hky_log_debug3(HKY_LOG_DEBUG_ALLOC,log,0,
                                                "memalign: %p:%uz @%uz",p,size,alignment);
    return p;
}
#endif
