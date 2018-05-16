#ifndef HKY_ALLOC_H_INCLUDED
#define HKY_ALLOC_H_INCLUDED

#include "../core/hky_config.h"
#include "../core/hky_core.h"

void *hky_alloc(size_t size,hky_log_t *log);
void *hky_calloc(size_t size,hky_log_t *log);

#define hky_free free

#if(HKY_HAVE_POSIX_MEMALIGN||HKY_HAVE_MEMALIGN)
void *hky_memalign(size_t alignment,size_t size,hky_log_t *log);
#else
#define hky_memalign(alignment,size,log)    hky_alloc(size,log);
#endif

//页面大小
//在hky_posix_init.c中设置
extern hky_uint_t hky_pagesize;
extern hky_uint_t hky_pagesize_shift;
extern hky_uint_t hky_cacheline_size;

#endif // HKY_ALLOC_H_INCLUDED
