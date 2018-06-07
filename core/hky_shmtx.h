#ifndef _HKY_SHMTX_H_INCLUDED_
#define _HKY_SHMTX_H_INCLUDED_

#include "hky_config.h"
#include "hky_core.h"

typedef struct {
	hky_atomic_t lock;
#if (HKY_HAVE_POSIX_SEM)
	hky_atomic_t wait;
#endif // (HKY_HAVE_POSIX_SEM)
}hky_shmtx_sh_t;

typedef struct {
#if (HKY_HAVE_ATOMIC_OPS)
	hky_atomic_t *lock;
#if (HKY_HAVE_POSIX_SEM)
	hky_atomic_t *wait;
	hky_uint_t semaphore;
	sem_t sem;
#endif // (HKY_HAVE_POSIX_SEM)
#else
	hky_fd_t	fd;
	hky_uchar	*name;
#endif
	hky_uint_t spin;
}hky_shmtx_t;

hky_int_t hky_shmtx_create(hky_shmtx_t *mtx, hky_shmtx_sh_t *addr, hky_uchar *name);
void hky_shmtx_destroy(hky_shmtx_t *mtx);

#endif // !_HKY_SHMTX_H_INCLUDED_
