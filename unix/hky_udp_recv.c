#pragma once
#include "../core/hky_config.h"
#include "../core/hky_core.h"
#include "../event/hky_event.h"

ssize_t 
hky_udp_unix_recv(hky_connection_t *c, hky_uchar *buf, size_t size) {
	ssize_t n;
	hky_err_t err;
	hky_event_t *rev;
	rev = c->read;
	do {
		n = recv(c->fd, buf, size, 0);
		hky_log_debug3(HKY_LOG_DEBUG_EVENT, c->log, 0,
			"recv: fd:%d %z of %uz", c->fd, n, size);
		if (n >= 0) {
#if (HKY_HAVE_KQUEUE)
			if (hky_event_flags&HKY_USE_KQUEUE_EVENT) {
				rev->available -= n;
				if (rev->available <= 0) {
					rev->ready = 0;
					rev->available = 0;
				}
			}
#endif // (HKY_HAVE_KQUEUE)
			return n;
		}
		err = hky_socket_errno;
		if (err == HKY_EAGAIN || err == HKY_EINTR) {
			hky_log_debug0(HKY_LOG_DEBUG_EVENT, c->log, err,
				"recv() not ready");
			n = HKY_AGAIN;
		}
		else {
			n = hky_connection_error(c, err, "recv() failed");
			break;
		}
	} while (err == HKY_EINTR);
	rev->ready = 0;
	if (n == HKY_ERROR) {
		rev->error = 1;
	}
	return n;
}