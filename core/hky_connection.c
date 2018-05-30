
#include "hky_config.h"
#include "hky_core.h"


hky_int_t hky_set_inherited_sockets(hky_cycle_t *cycle) {
	size_t len;
	hky_uint_t i;
	hky_listening_t *ls;
	socklen_t olen;
#if(HKY_HAVE_DEFERRED_ACCEPT||HKY_HAVE_TCP_FASTOPEN)
	hky_err_t err;
#endif
#if(HKY_HAVE_DEFERRED_ACCEPT||defined SO_ACCEPTFILTER)
	struct accept_filter_arg af;
#endif
#if(HKY_HAVE_DEFERRED_ACCEPT ||defined TCP_DEFER_ACCEPT)
	int timeout;
#endif
#if(HKY_HAVE_REUSEPORT)
	int reuseport;
#endif
	ls = cycle->listening.elts;
	for (i = 0; i < cycle->listening.nelts; i++) {
		ls[i].sockaddr = hky_palloc(cycle->pool, sizeof(hky_sockaddr_t));
		if (ls[i].sockaddr == NULL) {
			return HKY_ERROR;
		}

		ls[i].socklen = sizeof(hky_sockaddr_t);
		if (getsockname(ls[i].fd, ls[i].sockaddr, &ls[i].socklen) == -1) {
			hky_log_error(HKY_LOG_CRIT, cycle->log, hky_socket_errno,
				"getsockname() of the inherited "
				"socket #%d failed", ls[i].fd);
			ls[i].ignore = 1;
			continue;
		}
		if (ls[i].socklen > (socklen_t)sizeof(hky_sockaddr_t)) {
			ls[i].socklen = sizeof(hky_sockaddr_t);
		}
		switch (ls[i].sockaddr->sa_family) {
#if(HKY_HAVE_INET6)
		case AF_INET6:
			ls[i].addr_text_max_len = HKY_INET6_ADDRSTRLEN;
			len = HKY_INET6_ADDRSTRLEN + sizeof("[]:65535") - 1;
			break;
#endif
#if(HKY_HAVE_UNIX_DOMAIN)
		case AF_UNIX:
			ls[i].addr_text_max_len = HKY_UNIX_ADDRSTRLEN;
			len = HKY_UNIX_ADDRSTRLEN;
			break;
#endif
		case AF_INET:
			ls[i].addr_text_max_len = HKY_INET_ADDRSTRLEN;
			len = HKY_INET_ADDRSTRLEN + sizeof(":65535") - 1;
			break;
		default:
			hky_log_error(HKY_LOG_CRIT, cycle->log, hky_socket_errno,
				"the inherited socket #%d has "
				"an unsupported protocol family", ls[i].fd);
			ls[i].ignore = 1;
			continue;
		}
		ls[i].addr_text.data = hky_pnalloc(cycle->pool, len);
		if (ls[i].addr_text.data == NULL) {
			return HKY_ERROR;
		}

		len = hky_sock_ntop(ls[i].sockaddr, ls[i].socklen,
			ls[i].addr_text.data, len, 1);
		if (len == 0) {
			return HKY_ERROR;
		}

		ls[i].addr_text.len = len;
		ls[i].backlog = HKY_LISTEN_BACKLOG;

		olen = sizeof(int);

		if (getsockopt(ls[i].fd, SOL_SOCKET, SO_TYPE, (void*)&ls[i].type, &olen)
			== -1) {
			hky_log_error(HKY_LOG_CRIT, cycle->log, hky_socket_errno,
				"getsockopt(SO_TYPE) %V failed", &ls[i].addr_text);
			ls[i].ignore = 1;
			continue;
		}

		olen = sizeof(int);

		if (getsockopt(ls[i].fd, SOL_SOCKET, SO_RCVBUF, (void*)&ls[i].rcvbuf,
			&olen) == -1) {
			hky_log_error(HKY_LOG_ALERT, cycle->log, hky_socket_errno,
				"getsockopt(SO_RCVBUF) %V failed, ignored", &ls[i].addr_text);
			ls[i].rcvbuf = -1;
		}

		olen = sizeof(int);

		if (getsockopt(ls[i].fd, SOL_SOCKET, SO_SNDBUF, (void *)&ls[i].sndbuf,
			&olen)
			== -1) {
			hky_log_error(HKY_LOG_ALERT, cycle->log, hky_socket_errno,
				"getsockopt(SO_SNDBUF) %V failed, ignored", &ls[i].addr_text);
			ls[i].sndbuf = -1;
		}
#if 0
#if(HKY_HAVE_SETFIB)
		olen = sizeof(int);
		if (getsockopt(ls[i].fd, SOL_SOCKET, SO_SETFIB, (void*)&ls[i].setfib, &olen)
			== -1) {
			hky_log_error(HKY_LOG_ALERT, cycle->log, hky_socket_errno,
				"getsockopt(SO_SETFIB) %V failed, ignored", &ls[i].addr_text);
			ls[i].setfib = -1;
		}
#endif
#endif // 0
#if(HKY_HAVE_REUSEPORT)
		reuseport = 0;
		olen = sizeof(int);

		if (getsockopt(ls[i].fd, SOL_SOCKET, SO_REUSEPORT, (void*)&reuseport, &olen)
			== -1) {
			hky_log_error(HKY_LOG_ALERT, cycle->log, hky_socket_errno,
				"getsockopt(SO_REUSEPORT) %V failed, ignored", &ls[i].addr_text);
		}
		else {
			ls[i].reuseport = reuseport ? 1 : 0;
		}
#endif
		if (ls[i].type != SOCK_STREAM) {
			continue;
		}
#if(HKY_HAVE_TCP_FASTOPEN)
		olen = sizeof(int);
		if (getsockopt(ls[i].fd, IPPROTO_TCP, TCP_FASTOPEN,
			(void*)&ls[i].fastopen, &olen)
			== -1) {
			err = hky_socket_errno;
			if (err != HKY_EOPNOTSUPP && err != HKY_ENOPROTOOPT) {
				hky_log_error(HKY_LOG_NOTICE, cycle->log, err,
					"getsockopt(IPPROTO_TCP) %V failed, ignored", &ls[i].addr_text);
			}
			ls[i].fastopen = -1;

		}
#endif

#if(HKY_HAVE_DEFERRED_ACCEPT && defined SO_ACCEPTFILTER)
		hky_memzero(&af, sizeof(struct accept_filter_arg));
		olen = sizeof(struct accept_filter_arg);

		if (getsockopt(ls[i].fd, SOL_SOCKET, SO_ACCEPTFILTER, &af, &olen) == -1) {
			err = hky_socket_errno;
			if (err == HKY_EINVAL) {
				continue;
			}
			hky_log_error(HKY_LOG_NOTICE, cycle->log, err,
				"getsockopt(SO_ACCEPTFILTER) for %V failed, ignored", &ls[i].addr_text);
			continue;
		}
		if (olen < sizeof(struct accept_filter_arg) || af.af_name[0] == '\0') {
			continue;
		}
		ls[i].accept_filter = hky_palloc(cycle->pool, 16);
		if (ls[i].accept_filter == NULL) {
			return HKY_ERROR;
		}
		(void)hky_cpystrn((hky_uchar*)ls[i].accept_filter,
			(hky_uchar*)af.af_name, 16);
#endif
#if(HKY_HAVE_DEFERRED_ACCEPT && defined TCP_DEFER_ACCEPT)
		timeout = 0;
		olen = sizeof(int);

		if (getsockopt(ls[i].fd, IPPROTO_TCP, TCP_DEFER_ACCEPT, &timeout, &olen) == -1) {
			err = hky_socket_errno;
			if (err == HKY_EOPNOTSUPP) {
				continue;
			}
			hky_log_error(HKY_LOG_NOTICE, cycle->log, err,
				"getsockopt(TCP_DEFER_ACCEPT) for %V failed, ignored", &ls[i].addr_text);
			continue;
		}
		if (olen < sizeof(int) || timeout == 0) {
			continue;
		}

		ls[i].deferred_accept = 1;
#endif
	}
	return HKY_OK;
}