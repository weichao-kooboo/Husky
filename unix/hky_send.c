#pragma once
#include "../core/hky_config.h"
#include "../core/hky_core.h"
#include "../event/hky_event.h"

ssize_t 
hky_unix_send(hky_connection_t *c, hky_uchar *buf, size_t size) {
	ssize_t n;
	hky_err_t err;
	hky_event_t *wev;

	wev = c->write;
#if (HKY_HAVE_KQUEUE)
	if ((hky_event_flags&HKY_USE_KQUEUE_EVENT) && wev->pending_eof) {
		(void)hky_connection_error(c, wev->kq_errno,
			"kevent() reported about an closed connection");
		wev->error = 1;
		return HKY_ERROR;
	}
#endif // (HKY_HAVE_KQUEUE)
	for (;;) {
		n = send(c->fd, buf, size, 0);
		hky_log_debug3(HKY_LOG_DEBUG_EVENT, c->log, 0,
			"send: fd:%d %z of %uz", c->fd, n, size);
		if (n > 0) {
			if (n < (ssize_t)size) {
				wev->ready = 0;
			}
			c->sent += n;
			return n;
		}
		err = hky_socket_errno;
		if (n == 0) {
			hky_log_error(HKY_LOG_ALERT, c->log, err, "send() returned zero");
			wev->ready = 0;
			return n;
		}
		if (err == HKY_EAGAIN || err == HKY_EINTR) {
			wev->ready = 0;
			hky_log_debug0(HKY_LOG_DEBUG_EVENT, c->log, err,
				"send() not ready");
			if (err == HKY_EAGAIN) {
				return HKY_AGAIN;
			}
		}
		else {
			wev->error = 1;
			(void)hky_connection_error(c, err, "send() failed");
			return HKY_ERROR;
		}
	}
}