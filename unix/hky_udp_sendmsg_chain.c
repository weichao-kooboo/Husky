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
	size_t total, size;
	hky_uchar *prev;
	hky_uint_t n, flush;
	hky_chain_t *cl;
	struct iovec *iov;

	cl = in;
	iov = NULL;
	prev = NULL;
	total = 0;
	n = 0;
	flush = 0;

	for (; in && !flush; in = in->next) {
		if (in->buf->flush || in->buf->last_buf) {
			flush = 1;
		}
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
		size = in->buf->last - in->buf->pos;

		if (prev == in->buf->pos) {
			iov->iov_len += size;
		}
		else {
			if (n == vec->nalloc) {
				hky_log_error(HKY_LOG_ALERT, log, 0,
					"too many parts in a datagram");
				return HKY_CHAIN_ERROR;
			}
			iov = &vec->iovs[n++];
			iov->iov_base = (void*)in->buf->pos;
			iov->iov_len = size;
		}
		prev = in->buf->pos + size;
		total += size;
	}
	if (!flush) {
#if (HKY_SUPPRESS_WARN)
		vec->size = 0;
		vec->count = 0;
#endif // (HKY_SUPPRESS_WARN)
		return cl;
	}
	vec->count = n;
	vec->size = total;
	return in;
}
static ssize_t 
hky_sendmsg(hky_connection_t *c, hky_iovec_t *vec) {
	ssize_t n;
	hky_err_t err;
	struct msghdr	msg;

#if (HKY_HAVE_MSGHDR_MSG_CONTROL)
#if (HKY_HAVE_IP_SENDSRCADDR)
	hky_uchar msg_control[CMSG_SPACE(sizeof(struct in_addr))];
#elif (HKY_HAVE_IP_PKTINFO)
	hky_uchar msg_control[CMSG_SPACE(sizeof(struct in_pktinfo))];
#endif // (HKY_HAVE_IP_SENDSRCADDR)

#if (HKY_HAVE_INET6&&HKY_HAVE_IPV6_RECVPKTINFO)
	hky_uchar	msg_control6[CMSG_SPACE(sizeof(struct in6_pktinfo))];
#endif // (HKY_HAVE_INET6&&HKY_HAVE_IPV6_RECVPKTINFO)

#endif // (HKY_HAVE_MSGHDR_MSG_CONTROL)
	hky_memzero(&msg, sizeof(struct msghdr));
	if (c->socklen) {
		msg.msg_name = c->sockaddr;
		msg.msg_namelen = c->socklen;
	}
	msg.msg_iov = vec->iovs;
	msg.msg_iovlen = vec->count;

#if (HKY_HAVE_MSGHDR_MSG_CONTROL)
	if (c->listening&&c->listening->wildcard&&c->local_sockaddr) {
#if (HKY_HAVE_IP_SENDSRCADDR)
		if (c->local_sockaddr->sa_family == AF_INET) {
			struct cmsghdr *cmsg;
			struct in_addr *addr;
			struct sockaddr_in *sin;

			msg.msg_control = &msg_control;
			msg.msg_controllen = sizeof(msg_control);

			cmsg = CMSG_FIRSTHDR(&msg);
			cmsg->cmsg_level = IPPROTO_IP;
			cmsg->cmsg_type = IP_SENDSRCADDR;
			cmsg->cmsg_len = CMSG_LEN(sizeof(struct in_addr));

			sin = (struct sockaddr_in *)c->local_sockaddr;
			addr = (struct in_addr *)CMSG_DATA(cmsg);
			*addr = sin->sin_addr;
		}
#elif	(HKY_HAVE_IP_PKTINFO)
		if (c->local_sockaddr->sa_family == AF_INET) {
			struct cmsghdr *cmsg;
			struct in_pktinfo *pkt;
			struct sockaddr_in *sin;

			msg.msg_control = &msg_control;
			msg.msg_controllen = sizeof(msg_control);

			cmsg.CMSG_FIRSTHDR(&msg);
			cmsg->cmsg_level = IPPROTO_IP;
			cmsg->cmsg_type = IP_PKTINFO;
			cmsg->cmsg_len = CMSG_LEN(sizeof(struct in_pktinfo));

			sin = (struct sockaddr_in *)c->local_sockaddr;

			pkt = (struct in_pktinfo *)CMSG_DATA(cmsg);
			hky_memzero(pkt, sizeof(struct in_pktinfo));
			pkt->ipi_spec_dst = sin->sin_addr;
		}
#endif // (HKY_HAVE_IP_SENDSRCADDR)
#if (HKY_HAVE_INET6&&HKY_HAVE_IPV6_RECVPKTINFO)
		if (c->local_sockaddr->sa_family == AF_INET6) {
			struct cmsghdr *cmsg;
			struct in6_pktinfo *pkt6;
			struct sockaddr_in6 *sin6;

			msg.msg_control = &msg_control6;
			msg.msg_controllen = sizeof(msg_control6);

			cmsg = CMSG_FIRSTHDR(&msg);
			cmsg->cmsg_level = IPPROTO_IPV6;
			cmsg->cmsg_type = IPV6_PKTINFO;
			cmsg->cmsg_len = CMSG_LEN(sizeof(struct in6_pktinfo));

			sin6 = (struct sockaddr_in6 *)c->local_sockaddr;

			pkt6 = (struct in6_pktinfo *)CMSG_DATA(cmsg);
			hky_memzero(pkt6, sizeof(struct in6_pktinfo));
			pkt6->ipi6_addr = sin6->sin6_addr;
		}
#endif // (HKY_HAVE_INET6&&HKY_HAVE_IPV6_RECVPKTINFO)
	}
#endif // (HKY_HAVE_MSGHDR_MSG_CONTROL)
eintr:
	n = sendmsg(c->fd, &msg, 0);
	hky_log_debug2(HKY_LOG_DEBUG_EVENT, c->log, 0,
		"sendmsg: %z of %uz", n, vec->size);

	if (n == -1) {
		err = hky_errno;
		switch (err)
		{
		case HKY_EAGAIN:
			hky_log_debug0(HKY_LOG_DEBUG_EVENT, c->log, err,
				"sendmsg() not ready");
			return HKY_AGAIN;
		case HKY_EINTR:
			hky_log_debug0(HKY_LOG_DEBUG_EVENT, c->log, err,
				"sendmsg() was interrupted");
			goto eintr;
		default:
			c->write->error = 1;
			hky_connection_error(c, err, "sendmsg() failed");
			return HKY_ERROR;
		}
	}
	return n;
}