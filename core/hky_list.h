#ifndef HKY_LIST_H_INCLUDED
#define HKY_LIST_H_INCLUDED

#include "hky_config.h"
#include "hky_core.h"

typedef struct hky_list_part_s hky_list_part_t;

struct hky_list_part_s{
	//当前元素的偏移量
    void *elts;
	//当前part的元素个数
    hky_uint_t nelts;
	//下一个part
    hky_list_part_t *next;
};

typedef struct{
	//part链表的尾指针
    hky_list_part_t *last;
	//part链表的头指针
    hky_list_part_t part;
	//每个part中元素的size
    size_t size;
	//每个part中元素的总个数
    hky_uint_t nalloc;
    hky_pool_t  *pool;
} hky_list_t;

hky_list_t *hky_list_create(hky_pool_t *pool,hky_uint_t n,size_t size);

static hky_inline hky_int_t
hky_list_init(hky_list_t *list,hky_pool_t *pool,hky_uint_t n,size_t size){
    list->part.elts=hky_palloc(pool,n*size);
    if(list->part.elts==NULL){
        return HKY_ERROR;
    }
    list->part.nelts=0;
    list->part.next=NULL;
    list->last=&list->part;
    list->size=size;
    list->nalloc=n;
    list->pool=pool;

    return HKY_OK;
}

void *hky_list_push(hky_list_t *list);

#endif // HKY_LIST_H_INCLUDED
