#pragma once
#include "../core/hky_config.h"
#include "../core/hky_core.h"
#include "../event/hky_event.h"

ssize_t 
hky_unix_recv(hky_connection_t *c, hky_uchar *buf, size_t size) {
	ssize_t n;
	hky_err_t err;
	hky_event_t *rev;

	rev = c->read;

#if (HKY_HAVE_KQUEUE)
	if (hky_event_flags&HKY_USE_KQUEUE_EVENT) {
		hky_log_debug3(HKY_LOG_DEBUG_EVENT, c->log, 0,
			"recv: eof:%d, avail%D, err:%d",
			rev->pending_eof, rev->available, rev->kq_errno);

		if (rev->available == 0) {
			if (rev->pending_eof) {
				rev->ready = 0;
				rev->eof = 1;

				if (rev->kq_errno) {
					rev->error = 1;
					hky_set_socket_errno(rev->kq_errno);

					return hky_connection_error(c, rev->kq_errno,
						"kevent() reported about an closed connection");
				}
				return 0;
			}
			else {
				rev->ready = 0;
				return HKY_AGAIN;
			}
		}
	}
#endif // (HKY_HAVE_KQUEUE)

#if (HKY_HAVE_EPOLLRDHUP)
	if (hky_event_flags &HKY_USE_EPOLL_EVENT) {
		hky_log_debug2(HKY_LOG_DEBUG_EVENT, c->log, 0,
			"recv: eof:%d, avail:%d",
			rev->pending_eof, rev->available);
		if (!rev->available && !rev->pending_eof) {
			rev->ready = 0;
			return HKY_AGAIN;
		}
	}
#endif // (HKY_HAVE_EPOLLRDHUP)

}