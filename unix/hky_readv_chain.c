#pragma once
#include "../core/hky_config.h"
#include "../core/hky_core.h"
#include "../event/hky_event.h"


ssize_t 
hky_readv_chain(hky_connection_t *c, hky_chain_t *chain, off_t limit) {
	hky_uchar *prev;
	ssize_t n, size;
	hky_err_t err;
	hky_array_t vec;
	hky_event_t *rev;
	struct iovec *iov, iovs[HKY_IOVS_PREALLOCATE];

	rev = c->read;
#if (HKY_HAVE_KQUEUE)
	if (hky_event_flags&HKY_USE_KQUEUE_EVENT) {
		hky_log_debug3(HKY_LOG_DEBUG_EVENT, c->log, 0,
			"readv: eof:%d, avail:%d, err:%d",
			rev->pending_eof, rev->available, rev->kq_errno);
		if (rev->available == 0) {
			if (rev->pending_eof) {
				rev->ready = 0;
				rev->eof = 1;
				hky_log_error(HKY_LOG_INFO, c->log, rev->kq_errno,
					"kevent() reported about an closed connection");
				if (rev->kq_errno) {
					rev->error = 1;
					hky_set_socket_errno(rev->kq_errno);
					return HKY_ERROR;
				}
				return 0;
			}
			else {
				return HKY_AGAIN;
			}
		}
	}
#endif // (HKY_HAVE_KQUEUE)
#if (HKY_HAVE_EPOLLRDHUP)
	if (hky_event_flags&HKY_USE_EPOLL_EVENT) {
		hky_log_debug2(HKY_LOG_DEBUG_EVENT, c->log, 0,
			"readv: eof:%d, avail:%d",
			rev->pending_eof, rev->available);
		if (!rev->available && !rev->pending_eof) {
			return HKY_AGAIN;
		}
	}
#endif // (HKY_HAVE_EPOLLRDHUP)
	prev = NULL;
	iov = NULL;
	size = 0;

	vec.elts = iovs;
	vec.nelts = 0;
	vec.size = sizeof(struct iovec);
	vec.nalloc = HKY_IOVS_PREALLOCATE;
	vec.pool = c->pool;

	while (chain) {
		n = chain->buf->end - chain->buf->last;
		if (limit) {
			if (size >= limit) {
				break;
			}
			if (size + n > limit) {
				n = (ssize_t)(limit - size);
			}
		}
		if (prev == chain->buf->last) {
			iov->iov_len += n;
		}
		else {
			if (vec.nelts >= IOV_MAX) {
				break;
			}
			iov = hky_array_push(&vec);
			if (iov == NULL) {
				return HKY_ERROR;
			}
			iov->iov_base = (void*)chain->buf->last;
			iov->iov_len = n;
		}
		size += n;
		prev = chain->buf->end;
		chain = chain->next;
	}
	hky_log_debug2(HKY_LOG_DEBUG_EVENT, c->log, 0,
		"readv: %ui,last:%uz", vec.nelts, iov->iov_len);

	do {
		n = readv(c->fd, (struct iovec*)vec.elts, vec, nelts);

		if (n == 0) {
			rev->ready = 0;
			rev->eof = 1;
#if (HKY_HAVE_KQUEUE)
			if (hky_event_flags&HKY_USE_KQUEUE_EVENT) {
				rev->available = 0;
			}
#endif // (HKY_HAVE_KQUEUE)
			return 0;
		}
		if (n > 0) {
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
				&& hky_use_epoll_rdhup) {
				if (n < size) {
					if (!rev->pending_eof) {
						rev->ready = 0;
					}
					rev->available = 0;
				}
				return n;
			}
#endif // (HKY_HAVE_EPOLLRDHUP)
			if (n < size && !(hky_event_flags&HKY_USE_GREEDY_EVENT)) {
				rev->ready = 0;
			}
			return n;
		}
		err = hky_socket_errno;
		if (err == HKY_EAGAIN || err == HKY_EINTR) {
			hky_log_debug0(HKY_LOG_DEBUG_EVENT, c->log, err,
				"readv() not ready");
			n = HKY_AGAIN;
		}
		else {
			n = hky_connection_error(c, err, "readv() failed");
			break;
		}
	} while (err = HKY_EINTR);
	rev->ready = 0;
	if (n == HKY_ERROR) {
		c->read->error = 1;
	}
	return n;
}