#ifndef HKY_BUF_H_INCLUDED
#define HKY_BUF_H_INCLUDED

#include "hky_config.h"
#include "hky_core.h"

typedef  void * hky_buf_tag_t;

typedef struct hky_buf_s hky_buf_t;

struct hky_buf_s{
    hky_uchar *pos;
    hky_uchar *last;
    off_t file_pos;
    off_t file_last;

    hky_uchar *start;
    hky_uchar *end;
    hky_buf_tag_t tag;
    hky_file_t *file;
    hky_buf_t *shadow;

    unsigned temporary:1;

    unsigned memory:1;

    unsigned mmap:1;
    unsigned recycled:1;
    unsigned in_file:1;
    unsigned flush:1;
    unsigned sync:1;
    unsigned last_buf:1;
    unsigned last_in_chain:1;
    unsigned last_shadow:1;
    unsigned temp_file:1;
    int num;
};

#define HKY_CHAIN_ERROR	(hky_chain_t *)HKY_ERROR

struct hky_chain_s{
    hky_buf_t *buf;
    hky_chain_t *next;
};

hky_buf_t *hky_create_temp_buf(hky_pool_t *pool,size_t size);

#define hky_buf_in_memory(b)	(b->temporary||b->memory||b->mmap)
#define hky_buf_in_memory_only(b)	(hky_buf_in_memory(b)&&!b->in_file)

#define hky_buf_special(b)							\
		((b->flush||b->last_buf||b->sync)			\
		&& !hky_buf_in_memory(b)&&!b->in_file)

#define hky_buf_sync_only(b)						\
		(b->sync									\
		&& !hky_buf_in_memory(b)&&!b->in_file&&!b->flush&&!b->last_buf)
#define hky_buf_size(b)								\
		(hky_buf_in_memory(b)?(off_t)(b->last-b->pos):				\
									(b->file_last-b->file_pos))
	

#define hky_alloc_buf(pool) hky_palloc(pool,sizeof(hky_buf_t))
#define hky_calloc_buf(pool) hky_pcalloc(pool,sizeof(hky_buf_t))

hky_chain_t *hky_chain_update_sent(hky_chain_t *in, off_t sent);

#endif // HKY_BUF_H_INCLUDED
