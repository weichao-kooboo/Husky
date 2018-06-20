
#include "hky_config.h"
#include "hky_core.h"

#define HKY_SLAB_PAGE_MASK	3
#define HKY_SLAB_PAGE		0
#define HKY_SLAB_BIG		1
#define HKY_SLAB_EXACT		2
#define HKY_SLAB_SMALL		3

#if (HKY_PTR_SIZE==4)

#define	HKY_SLAB_PAGE_FREE	0
#define HKY_SLAB_PAGE_BUSY	0xffffffff
#define HKY_SLAB_PAGE_START	0x80000000

#define	HKY_SLAB_SHIFT_MASK	0x0000000f
#define	HKY_SLAB_MAP_MASK	0xffff0000
#define HKY_SLAB_MAP_SHIFT	16

#define HKY_SLAB_BUSY		0xffffffff
#else

#define	HKY_SLAB_PAGE_FREE	0
#define HKY_SLAB_PAGE_BUSY	0xffffffffffffffff
#define HKY_SLAB_PAGE_START	0x8000000000000000

#define	HKY_SLAB_SHIFT_MASK	0x000000000000000f
#define	HKY_SLAB_MAP_MASK	0xffffffff00000000
#define HKY_SLAB_MAP_SHIFT	32

#define HKY_SLAB_BUSY		0xffffffffffffffff

#endif // (HKY_PTR_SIZE==4)


#define hky_slab_slots_(pool)											\
		(hky_slab_page_t*)((hky_uchar*)(pool)+sizeof(hky_slab_pool_t))

#define hky_slab_page_type(page)	((page)->prev&HKY_SLAB_PAGE_MASK)
#define hky_slab_page_prev(page)										\
		(hky_slab_page_t *)((page)->prev&~HKY_SLAB_PAGE_MASK)
#define hky_slab_page_addr(pool,page)									\
		((((page)-(pool)->pages)<<hky_pagesize_shift)					\
		+(uintptr_t)(pool)->start)

#if (HKY_DEBUG_MALLOC)

#define	hky_slab_junk(p,size)	hky_memset(p,0xA5,size)

#elif (HKY_HAVE_DEBUG_MALLOC)

#define hky_slab_junk(p,size)											\
		if(hky_debug_malloc)	hky_memset(p,0xA5,size)

#else
#define hky_slab_junk(p,size)
#endif // (HKY_DEBUG_MALLOC)


void 
hky_slab_init(hky_slab_pool_t *pool) {
	hky_uchar *p;
	size_t size;
	hky_int_t m;
	hky_uint_t i, n, pages;
	hky_slab_page_t *slots, *page;

	pool->min_size = (size_t)1 << pool->min_shift;

	slots = hky_slab_slots(pool);
	p = (hky_uchar*)slots;
	size = pool->end - p;

	hky_slab_junk(p, size);
	n = hky_pagesize_shift - pool->min_shift;

	for (i = 0; i < n; i++) {
		slots[i].slab = 0;
		slots[i].next = &slots[i];
		slots[i].prev = 0;
	}

	p += n * sizeof(hky_slab_page_t);

	pool->stats = (hky_slab_stat_t*)p;
	hky_memzero(pool->stats, n * sizeof(hky_slab_stat_t));

	p += n * sizeof(hky_slab_stat_t);
	size -= n * (sizeof(hky_slab_page_t) + sizeof(hky_slab_stat_t));

	pages = (hky_uint_t)(size / (hky_pagesize + sizeof(hky_slab_page_t)));

	pool->pages = (hky_slab_page_t *)p;
	hky_memzero(pool->pages, pages * sizeof(hky_slab_page_t));

	page = pool->pages;

	pool->free.slab = 0;
	pool->free.next = page;
	pool->free.prev = 0;

	page->slab = pages;
	page->next = &pool->free;
	page->prev = (uintptr_t)&pool->free;

	pool->start = hky_align_ptr(p + pages * sizeof(hky_slab_page_t), hky_pagesize);
	m = pages - (pool->end - pool->start) / hky_pagesize;
	if (m > 0) {
		pages -= m;
		page->slab = pages;
	}
	pool->last = pool->pages + pages;
	pool->pfree = pages;

	pool->log_nomem = 1;
	pool->log_ctx = &pool->zero;
	pool->zero = '\0';
}