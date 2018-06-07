
#include "hky_config.h"
#include "hky_core.h"

void 
hky_slab_init(hky_slab_pool_t *pool) {
	hky_uchar *p;
	size_t size;
	hky_int_t m;
	hky_uint_t i, n, pages;
	hky_slab_page_t *slots, *page;

	pool->min_size = (size_t)1 << pool->min_shift;

	slots=hky_slab_stat_t
}