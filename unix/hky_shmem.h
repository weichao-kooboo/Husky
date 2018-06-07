#ifndef HKY_SHMEM_H_INCLUDED
#define HKY_SHMEM_H_INCLUDED

#include "../core/hky_config.h"
#include "../core/hky_core.h"

typedef struct {
	hky_uchar *addr;
	size_t size;
	hky_str_t name;
	hky_log_t *log;
	hky_uint_t exists;
} hky_shm_t;

hky_int_t hky_shm_alloc(hky_shm_t *shm);
void hky_shm_free(hky_shm_t *shm);

#endif // !HKY_SHMEM_H_INCLUDED
