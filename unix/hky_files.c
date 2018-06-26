
#include "../core/hky_config.h"
#include "../core/hky_core.h"

ssize_t
hky_read_file(hky_file_t *file, hky_uchar *buf, size_t size, off_t offset) {
	ssize_t n;
	hky_log_debug4(HKY_LOG_DEBUG_CORE, file->log, 0,
		"read: %d, %p, %uz, %0", file->fd, buf, size, offset);

#if(HKY_HAVE_PREAD)
	n = pread(file->fd, buf, size, offset);
	if (n == -1) {
		hky_log_error(HKY_LOG_CRIT, file->log, hky_errno,
			"pread() \"%s\" failed", file->name.data);
		return HKY_ERROR;
	}
#else
	if (file->sys_offset != offset) {
		if (lseek(file->fd, offset, SEEK_SET) == -1) {
			hky_log_error(HKY_LOG_CRIT, file->log, hky_errno,
				"lseek() \"%s\" failed", file->name.data);
			return HKY_ERROR;
		}
		file->sys_offset = offset;
	}

	n = read(file->fd, buf, size);

	if (n == -1) {
		hky_log_error(HKY_LOG_CRIT, file->log, hky_errno,
			"read() \"%s\" failed", file->name.data);
		return HKY_ERROR;
	}
	file->sys_offset += n;
#endif

	file->offset += n;
	return	n;
}

ssize_t
hky_write_file(hky_file_t *file, hky_uchar *buf, size_t size, off_t offset) {
	ssize_t n, written;
	hky_err_t err;

	hky_log_debug4(HKY_LOG_DEBUG_CORE, file->log, 0,
		"write:%d,%p,%uz,%0", file->fd, buf, size, offset);

	written = 0;
#if (HKY_HAVE_PWRITE)
	for (;;) {
		n = pwrite(file->fd, buf + written, size, offset);
		if (n == -1) {
			err = hky_errno;
			if (err == HKY_EINTR) {
				hky_log_debug0(HKY_LOG_DEBUG_CORE, file->log, err,
					"pwrite() was interrupted");
				continue;
			}
			hky_log_error(HKY_LOG_CRIT, file->log, err,
				"pwrite() \"%s\" failed", file->name.data);
			return HKY_ERROR;
		}
		file->offset += n;
		written += n;
		if ((size_t)n == size) {
			return written;
		}
		offset += n;
		size -= n;
	}
#else
	if (file->sys_offset != offset) {
		if (lseek(file->fd, offset, SEEK_SET) == -1) {
			hky_log_error(HKY_LOG_CRIT, file->log, hky_errno,
				"lseek() \"%s\" failed", file->name.data);
			return HKY_ERROR;
		}
		file->sys_offset = offset;
	}
	for (;;) {
		n = write(file->fd, buf + written, size);
		if (n == -1) {
			err = hky_errno;
			if (err == HKY_EINTR) {
				hky_log_debug0(HKY_LOG_DEBUG_CORE, file->log, err,
					"write() was interrupted");
				continue;
			}
			hky_log_error(HKY_LOG_CRIT, file->log, err,
				"write() \"%s\" failed", file->name.data);
			return HKY_ERROR;
		}
		file->sys_offset += n;
		file->offset += n;
		written += n;
		if ((size_t)n == size) {
			return written;
		}
		size -= n;
	}
#endif // (HKY_HAVE_PWRITE)
}