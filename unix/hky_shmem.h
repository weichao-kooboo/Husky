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

#endif // !HKY_SHMEM_H_INCLUDED
