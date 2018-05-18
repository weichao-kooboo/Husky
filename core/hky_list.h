#ifndef HKY_LIST_H_INCLUDED
#define HKY_LIST_H_INCLUDED

#include "hky_config.h"
#include "hky_core.h"

typedef struct hky_list_part_s hky_list_part_t;

struct hky_list_part_s{
	//��ǰԪ�ص�ƫ����
    void *elts;
	//��ǰpart��Ԫ�ظ���
    hky_uint_t nelts;
	//��һ��part
    hky_list_part_t *next;
};

typedef struct{
	//part�����βָ��
    hky_list_part_t *last;
	//part�����ͷָ��
    hky_list_part_t part;
	//ÿ��part��Ԫ�ص�size
    size_t size;
	//ÿ��part��Ԫ�ص��ܸ���
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
