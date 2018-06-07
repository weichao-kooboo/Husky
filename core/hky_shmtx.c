
#include "hky_config.h"
#include "hky_core.h"

#if (HKY_HAVE_ATOMIC_OPS)
hky_int_t 
hky_shmtx_create(hky_shmtx_t *mtx, hky_shmtx_sh_t *addr, hky_uchar *name) {
	mtx->lock = &addr->lock;
	if (mtx->spin == (hky_uint_t)-1) {
		return HKY_OK;
	}
	mtx->spin = 2048;
#if (HKY_HAVE_POSIX_SEM)
	mtx->wait = &addr->wait;
	if (sem_init(&mtx->sem, 1, 0) == -1) {
		hky_log_error(HKY_LOG_ALERT, hky_cycle->log, hky_errno,
			"sem_init() failed");
	}
	else {
		mtx->semaphore = 1;
	}
#endif // (HKY_HAVE_POSIX_SEM)
	return HKY_OK;
}
void
hky_shmtx_destroy(hky_shmtx_t *mtx) {
#if (HKY_HAVE_POSIX_SEM)
	if (mtx->semaphore) {
		if (sem_destroy(&mtx->sem) == -1) {
			hky_log_error(HKY_LOG_ALERT, hky_cycle->log, hky_errno,
				"sem_destroy() failed");
		}
	}
#endif // (HKY_HAVE_POSIX_SEM)

}
#else
hky_int_t 
hky_shmtx_create(hky_shmtx_t *mtx, hky_shmtx_sh_t *addr, hky_uchar *name) {
	if (mtx->name) {
		if (hky_strcmp(name, mtx->name) == 0) {
			mtx->name = name;
			return HKY_OK;
		}
		hky_shmtx_destroy(mtx);
	}
	mtx->fd = hky_open_file(name, HKY_FILE_RDWR, HKY_FILE_CREATE_OR_OPEN,
		HKY_FILE_DEFAULT_ACCESS);

	if (mtx->fd == HKY_INVALID_FILE) {
		hky_log_error(HKY_LOG_EMERG, hky_cycle->log, hky_errno,
			hky_open_file_n "\"%s\" failed", name);
		return HKY_ERROR;
	}
	if (hky_delete_file(name) == HKY_FILE_ERROR) {
		hky_log_error(HKY_LOG_ALERT, hky_cycle->log, hky_errno,
			hky_delete_file_n "\"%s\" failed", name);
	}
	mtx->name = name;
	return HKY_OK;
}
void 
hky_shmtx_destroy(hky_shmtx_t *mtx) {
	if (hky_close_file(mtx->fd) == HKY_FILE_ERROR) {
		hky_log_error(HKY_LOG_ALERT, hky_cycle->log, hky_errno,
			hky_close_file_n "\"%s\" failed", mtx->name);
	}
}
#endif // (HKY_HAVE_ATOMIC_OPS)
