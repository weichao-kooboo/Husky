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
	do {
		n = recv(c->fd, buf, size, 0);
		hky_log_debug3(HKY_LOG_DEBUG_EVENT, c->log, 0,
			"recv: fd:%d %z of %uz", c->fd, n, size);

		if (n == 0)
		{
			rev->ready = 0;
			rev->eof = 1;
#if (HKY_HAVE_KQUEUE)
			if (hky_event_flags&HKY_USE_KQUEUE_EVENT) {
				rev->available = 0;
			}
#endif // (HKY_HAVE_KQUEUE)
			return 0;
		}
		if (n > 0)
		{
#if (HKY_HAVE_KQUEUE)
			if (hky_event_flags&HKY_USE_KQUEUE_EVENT) {
				rev->available -= n;
				if (rev->available <= 0) {
					if (!rev->pending_eof) {
						rev->ready = 0;
					}
					rev->available = 0;
				}
				return n;
			}
#endif // (HKY_HAVE_KQUEUE)
#if (HKY_HAVE_EPOLLRDHUP)
			if ((hky_event_flags&HKY_USE_EPOLL_EVENT)
				&& hky_use_epoll_rdhup){
				if ((size_t)n < size) {
					if (!rev->pending_eof) {
						rev->ready = 0;
					}
					rev->available = 0;
				}
				return n;
			}
#endif // (HKY_HAVE_EPOLLRDHUP)
			if ((size_t)n < size
				&& !(hky_event_flags&HKY_USE_GREEDY_EVENT)) {
				rev->ready = 0;
			}
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