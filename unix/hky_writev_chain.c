#pragma once

#include "../core/hky_config.h"
#include "../core/hky_core.h"
#include "../event/hky_event.h"

hky_chain_t *
hky_writev_chain(hky_connection_t *c, hky_chain_t *in, off_t limit) {
	ssize_t n, sent;
	off_t send, prev_send;
	hky_chain_t *cl;
	hky_event_t *wev;
	hky_iovec_t vec;
	struct iovec iovs[HKY_IOVS_PREALLOCATE];

	wev = c->write;
	if (!wev->ready) {
		return in;
	}

#if (HKY_HAVE_KQUEUE)
	if ((hky_event_flags&HKY_USE_KQUEUE_EVENT) && wev->pending_eof) {
		(void)hky_connection_error(c, wev->kq_errno,
			"kevent() reported about an closed connection");
		wev->error = 1;
		return HKY_CHAIN_ERROR;
	}
#endif // (HKY_HAVE_KQUEUE)

	if (limit == 0 || limit > (off_t)(HKY_MAX_SIZE_T_VALUE - hky_pagesize)) {
		limit = HKY_MAX_SIZE_T_VALUE - hky_pagesize;
	}
	send = 0;
	vec.iovs = iovs;
	vec.nalloc = HKY_IOVS_PREALLOCATE;

	for (;;) {
		prev_send = send;
		cl = hky_output_chain_to_iovec(&vec, in, limit - send, c->log);
		if (cl == HKY_CHAIN_ERROR) {
			return HKY_CHAIN_ERROR;
		}
		if (cl&&cl->buf->in_file) {
			hky_log_error(HKY_LOG_ALERT, c->log, 0,
				"file buf in writev "
				"t:%d r:%d f:%d %p %p-%p %p %O-%O",
				cl->buf->temporary,
				cl->buf->recycled,
				cl->buf->in_file,
				cl->buf->start,
				cl->buf->pos,
				cl->buf->last,
				cl->buf->file,
				cl->buf->file_pos,
				cl->buf->file_last);
			hky_debug_point();

			return HKY_CHAIN_ERROR;
		}
		send += vec.size;

		n = hky_writev(c, &vec);
		if (n == HKY_ERROR) {
			return HKY_CHAIN_ERROR;
		}
		sent = (n == HKY_AGAIN) ? 0 : n;
		c->sent += sent;

		in = hky_chain_update_sent(in, sent);
		if (send - prev_send != sent) {
			wev->ready = 0;
			return in;
		}
		if (send >= limit || in == NULL) {
			return in;
		}
	}
}

hky_chain_t *
hky_output_chain_to_iovec(hky_iovec_t *vec, hky_chain_t *in,
	size_t limit, hky_log_t *log) {
	size_t total, size;
	hky_uchar	*prev;
	hky_uint_t	n;
	struct iovec *iov;

	iov = NULL;
	prev = NULL;
	total = 0;
	n = 0;

	for (; in&&total < limit; in = in->next) {
		if (hky_buf_special(in->buf)) {
			continue;
		}
		if (in->buf->in_file) {
			break;
		}
		if (!hky_buf_in_memory(in->buf)) {
			hky_log_error(HKY_LOG_ALERT, log, 0,
				"bad buf in output chain "
				"t:%d r:%d f:%d %p %p-%p %p %O-%O",
				in->buf->temporary,
				in->buf->recycled,
				in->buf->in_file,
				in->buf->start,
				in->buf->pos,
				in->buf->last,
				in->buf->file,
				in->buf->file_pos,
				in->buf->file_last);
			hky_debug_point();
			return HKY_CHAIN_ERROR;
		}
		size = in->buf->last - in->buf->pos;
		if (size > limit - total) {
			size = limit - total;
		}
		if (prev == in->buf->pos) {
			iov->iov_len += size;
		}
		else {
			if (n == vec->nalloc) {
				break;
			}
			iov = &vec->iovs[n++];
			iov->iov_base = (void*)in->buf->pos;
			iov->iov_len = size;
		}
		prev = in->buf->pos + size;
		total += size;
	}
	vec->count = n;
	vec->size = total;

	return in;
}

ssize_t 
hky_writev(hky_connection_t *c, hky_iovec_t *vec) {
	ssize_t n;
	hky_err_t err;
eintr:
	n = writev(c->fd, vec->iovs, vec->count);
	hky_log_debug2(HKY_LOG_DEBUG_EVENT, c->log, 0,
		"writev: %z of %uz", n, vec->size);

	if (n == -1) {
		err = hky_errno;
		switch (err)
		{
		case HKY_EAGAIN:
			hky_log_debug0(HKY_LOG_DEBUG_EVENT, c->log, err,
				"writev() not ready");
			return HKY_AGAIN;
		case HKY_EINTR:
			hky_log_debug0(HKY_LOG_DEBUG_EVENT, c->log, err,
				"writev() was interrupted");
			goto eintr;
		default:
			c->write->error = 1;
			hky_connection_error(c, err, "writev() failed");
			return HKY_ERROR;
		}
	}
	return n;
}