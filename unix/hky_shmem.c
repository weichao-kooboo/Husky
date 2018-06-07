#include "../core/hky_config.h"
#include "../core/hky_core.h"

#if (HKY_HAVE_MAP_ANON)

hky_int_t 
hky_shm_alloc(hky_shm_t *shm) {
	shm->addr = (hky_uchar*)mmap(NULL, shm->size,
		PROT_READ | PROT_WRITE,
		MAP_ANON | MAP_SHARED, -1, 0);
	if (shm->addr == MAP_FAILED) {
		hky_log_error(HKY_LOG_ALERT, shm->log, hky_errno,
			"mmap(MAP_ANON|MAP_SHARED, %uz) failed", shm->size);
		return HKY_ERROR;
	}
	return	HKY_OK;
}
void 
hky_shm_free(hky_shm_t *shm) {
	if (munmap((void*)shm->addr, shm->size) == -1) {
		hky_log_error(HKY_LOG_ALERT, shm->log, hky_errno,
			"munmap(%p,%uz) failed", shm->addr, shm->size);
	}
}

#elif(HKY_HAVE_MAP_DEVZERO)

hky_int_t 
hky_shm_alloc(hky_shm_t *shm) {
	hky_fd_t fd;
	fd = open("/dev/zero", O_RDWR);

	if (fd == -1) {
		hky_log_error(HKY_LOG_ALERT, shm->log, hky_errno,
			"open(\"/dev/zero\") failed");
		return HKY_ERROR;
	}
	shm->addr = (hky_uchar*)mmap(NULL, shm->size, PROT_READ | PROT_WRITE,
		MAP_SHARED, fd, 0);

	if (shm->addr == MAP_FAILED) {
		hky_log_error(HKY_LOG_ALERT, shm->log, hky_errno,
			"mmap(/dev/zero,MAP_SHARED,%uz) failed", shm->size);
	}
	if (close(fd) == -1) {
		hky_log_error(HKY_LOG_ALERT, shm->log, hky_errno,
			"close(\"/dev/zero\") failed");
	}
	return (shm->addr == MAP_FAILED) ? HKY_ERROR : HKY_OK;
}
void 
hky_shm_free(hky_shm_t *shm) {
	if (munmap((void*)shm->addr, shm->size) == -1) {
		hky_log_error(HKY_LOG_ALERT, shm->log, hky_errno,
			"munmap(%p,%uz) failed", shm->addr, shm->size);
	}
}

#elif(HKY_HAVE_SYSVSHM)

#include <sys/ipc.h>
#include <sys/shm.h>

hky_int_t 
hky_shm_alloc(hky_shm_t *shm) {
	int id;
	id = shmget(IPC_PRIVATE, shm->size, (SHM_R | SHM_W | IPC_CREAT));

	if (id == -1) {
		hky_log_error(HKY_LOG_ALERT, shm->log, hky_errno,
			"shmget(%uz) failed", shm->size);
		return HKY_ERROR;
	}
	hky_log_debug1(HKY_LOG_DEBUG_CORE, shm->log, 0, "shmget id: %d", id);

	shm->addr = shmat(id, NULL, 0);

	if (shm->addr == (void*)-1) {
		hky_log_error(HKY_LOG_ALERT, shm->log, hky_errno, "shmat() failed");
	}
	if (shmctl(id, IPC_RMID, NULL) == -1) {
		hky_log_error(HKY_LOG_ALERT, shm->log, hky_errno,
			"shmctl(IPC_RMID) failed");
	}

	return (shm->addr == (void*)-1) ? HKY_ERROR : HKY_OK;
}
void 
hky_shm_free(hky_shm_t *shm) {
	if (shmdt(shm->addr) == -1) {
		hky_log_error(HKY_LOG_ALERT, shm->log, hky_errno,
			"shmdt(%p) failed", shm->addr);
	}
}

#endif // (HKY_HAVE_MAP_ANON)
