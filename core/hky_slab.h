#ifndef _HKY_SLAB_H_INCLUDED_
#define	_HKY_SLAB_H_INCLUDED_

#include "hky_config.h"
#include "hky_core.h"

typedef struct hky_slab_page_s hky_slab_page_t;

struct hky_slab_page_s {
	uintptr_t slab;
	hky_slab_page_t *next;
	uintptr_t prev;
};

typedef	struct {
	hky_uint_t total;
	hky_uint_t used;
	hky_uint_t reqs;
	hky_uint_t fails;
}hky_slab_stat_t;

typedef struct {
	size_t min_size;
	size_t min_shift;

	hky_slab_page_t *pages;
	hky_slab_page_t *last;
	hky_slab_page_t free;

	hky_slab_stat_t *stats;
	hky_uint_t pfree;

	hky_uchar *start;
	hky_uchar *end;

	hky_uchar *log_ctx;
	hky_uchar zero;

	unsigned log_nomem : 1;

	void *data;
	void *addr;
}hky_slab_pool_t;

#endif // !_HKY_SLAB_H_INCLUDED_

