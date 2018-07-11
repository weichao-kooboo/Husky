#pragma once

#include "../core/hky_config.h"
#include "../core/hky_core.h"
#include "../event/hky_event.h"

static hky_chain_t *hky_udp_output_chain_to_iovec(hky_iovec_t *vec,
	hky_chain_t *in, hky_log_t *log);
static ssize_t hky_sendmsg(hky_connection_t *c, hky_iovec_t *vec);

hky_chain_t *
hky_udp_unix_sendmsg_chain(hky_connection_t *c, hky_chain_t *in, off_t limit) {
	ssize_t n;
	off_t send;
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
		cl = hky_udp_output_chain_to_iovec(&vec, in, c->log);
		if (cl == HKY_CHAIN_ERROR) {
			return HKY_CHAIN_ERROR;
		}
		if (cl&&cl->buf->in_file) {
			hky_log_error(HKY_LOG_ALERT, c->log, 0,
				"file buf in sendmsg "
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
		if (cl == in) {
			return in;
		}
		send += vec.size;
		n = hky_sendmsg(c, &vec);
		if (n == HKY_ERROR) {
			return HKY_CHAIN_ERROR;
		}
		if (n == HKY_AGAIN) {
			wev->ready = 0;
			return in;
		}
		c->sent += n;
		in = hky_chain_update_sent(in, n);
		if (send >= limit || in == NULL) {
			return in;
		}
	}
}

static hky_chain_t *
hky_udp_output_chain_to_iovec(hky_iovec_t *vec,
	hky_chain_t *in, hky_log_t *log) {
    
}
static ssize_t 
hky_sendmsg(hky_connection_t *c, hky_iovec_t *vec) {
    
}