
#include "hky_config.h"
#include "hky_core.h"
#include "../event/hky_event.h"


hky_int_t 
hky_set_inherited_sockets(hky_cycle_t *cycle) {
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
hky_int_t 
hky_open_listening_sockets(hky_cycle_t *cycle) {
	int reuseaddr;
	hky_uint_t i, tries, failed;
	hky_err_t err;
	hky_log_t *log;
	hky_socket_t s;
	hky_listening_t *ls;

	reuseaddr = 1;
#if (HKY_SUPPRESS_WARN)
	failed = 0;
#endif // (HKY_SUPPRESS_WARN)
	log = cycle->log;

	for (tries = 5; tries; tries--) {
		failed = 0;

		ls = cycle->listening.elts;
		for (i = 0; i < cycle->listening.nelts; i++) {
			if (ls[i].ignore) {
				continue;
			}
#if (HKY_HAVE_REUSEPORT)
			if (ls[i].add_reuseport) {
				int reuseport = 1;
				if (setsockopt(ls[i].fd, SOL_SOCKET, SO_REUSEPORT,
					(const void *)&reuseport, sizeof(int))
					== -1) {
					hky_log_error(HKY_LOG_ALERT, cycle->log, hky_socket_errno,
						"setsockopt(SO_REUSEPORT) %V failed,ignored", &ls[i].addr_text);
				}
				ls[i].add_reuseport = 0;
			}
#endif // (HKY_HAVE_REUSEPORT)
			if (ls[i].fd != (hky_socket_t)-1) {
				continue;
			}
			if (ls[i].inherited) {
				continue;
			}
			s = hky_socket(ls[i].sockaddr->sa_family, ls[i].type, 0);
			if (s == (hky_socket_t)-1) {
				hky_log_error(HKY_LOG_EMERG, log, hky_socket_errno,
					hky_socket_n " %V failed", &ls[i].addr_text);
				return HKY_ERROR;
			}
			if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR,
				(const void *)&reuseaddr, sizeof(int))
				== -1) {
				hky_log_error(HKY_LOG_EMERG, log, hky_socket_errno,
					"setsockopt(SO_REUSEADDR) %V failed",
					&ls[i].addr_text);
				if (hky_close_socket(s) == -1) {
					hky_log_error(HKY_LOG_EMERG, log, hky_socket_errno,
						hky_close_socket_n "%V failed",
						&ls[i].addr_text);
				}
				return HKY_ERROR;
			}
#if (HKY_HAVE_REUSEPORT)
			if (ls[i].reuseport && !hky_test_config) {
				int reuseport;

				reuseport = 1;
				if (setsockopt(s, SOL_SOCKET, SO_REUSEPORT,
					(const void *)&reuseport, sizeof(int))
					== -1) {
					hky_log_error(HKY_LOG_EMERG, log, hky_socket_errno,
						"setsockopt(SO_REUSEPORT) %V failed",
						&ls[i].addr_text);
					if (hky_close_socket(s) == -1) {
						hky_log_error(HKY_LOG_EMERG, log, hky_socket_errno,
							hky_close_socket_n " %V failed",
							&ls[i].addr_text);
					}
					return HKY_ERROR;
				}
			}
#endif // (HKY_HAVE_REUSEPORT)

#if (HKY_HAVE_INET6&& defined IPV6_V6ONLY)
			if (ls[i].sockaddr->sa_family == AF_INET6) {
				int ipv6only;
				ipv6only = ls[i].ipv6only;
				if (setsockopt(s, IPPROTO_IPV6, IPV6_V6ONLY,
					(const void *)&ipv6only, sizeof(int))
					== -1) {
					hky_log_error(HKY_LOG_EMERG, log, hky_socket_errno,
						"setsockopt(IPV6_V6ONLY) %V failed,ignored",
						&ls[i].addr_text);
				}
			}
#endif // (HKY_HAVE_INET6&& defined IPV6_V6ONLY)
			if (!(hky_event_flags&HKY_USE_IOCP_EVENT)) {
				if (hky_nonblocking(s) == -1) {
					hky_log_error(HKY_LOG_EMERG, log, hky_socket_errno,
						hky_nonblocking_n " %V failed",
						&ls[i].addr_text);

					if (hky_close_socket(s) == -1) {
						hky_log_error(HKY_LOG_EMERG, log, hky_socket_errno,
							hky_close_socket_n " %V failed",
							&ls[i].addr_text);
					}
					return HKY_ERROR;
				}
			}
			hky_log_debug2(HKY_LOG_DEBUG_CORE, log, 0
				"bind() %V #%d", &ls[i].addr_text, s);

			if (bind(s, ls[i].sockaddr, ls[i].socklen) == -1) {
				err = hky_socket_errno;

				if (err != HKY_EADDRINUSE || !hky_test_config) {
					hky_log_error(HKY_LOG_EMERG, log, err,
						"bind() to %V failed", &ls[i].addr_text);
				}
				if (hky_close_socket(s) == -1) {
					hky_log_error(HKY_LOG_EMERG, log, hky_socket_errno,
						hky_close_socket_n " %V failed",
						&ls[i].addr_text);
				}
				if (err != HKY_EADDRINUSE) {
					return HKY_ERROR;
				}
				if (!hky_test_config) {
					failed = 1;
				}
				continue;
			}
#if (HKY_HAVE_UNIX_DOMAIN)
			if (ls[i].sockaddr->sa_family == AF_UNIX) {
				mode_t mode;
				hky_uchar *name;

				name = ls[i].addr_text.data + sizeof("unix:") - 1;
				mode = (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

				if (chmod((char *)name, mode) == -1) {
					hky_log_error(HKY_LOG_EMERG, cycle->log, hky_errno,
						"chmod() \"%s\" failed", name);
				}
				if (hky_test_config) {
					if (hky_delete_file(name) == HKY_FILE_ERROR) {
						hky_log_error(HKY_LOG_EMERG, cycle->log, hky_errno,
							hky_delete_file_n "%s failed", name);
					}
				}
			}
#endif // (HKY_HAVE_UNIX_DOMAIN)
			if (ls[i].type != SOCK_STREAM) {
				ls[i].fd = s;
				continue;
			}
			if (listen(s, ls[i].backlog) == -1) {
				err = hky_socket_errno;

				if (err != HKY_EADDRINUSE || !hky_test_config) {
					hky_log_error(HKY_LOG_EMERG, log, err,
						"listen() to %V,backlog %d failed",
						&ls[i].addr_text, ls[i].backlog);
				}
				if (hky_close_socket(s) == -1) {
					hky_log_error(HKY_LOG_EMERG, log, hky_socket_errno,
						hky_close_socket_n " %V failed",
						&ls[i].addr_text);
				}
				if (err != HKY_EADDRINUSE) {
					return HKY_ERROR;
				}
				if (!hky_test_config) {
					failed = 1;
				}
				continue;
			}
			ls[i].listen = 1;
			ls[i].fd = s;
		}
		if (!failed) {
			break;
		}

		hky_log_error(HKY_LOG_NOTICE, log, 0,
			"try again to bind() after 500ms");
		hky_msleep(500);
	}
	if (failed) {
		hky_log_error(HKY_LOG_EMERG, log, 0, "still could not bind()");
		return HKY_ERROR;
	}
	return HKY_OK;
}

void 
hky_configure_listening_sockets(hky_cycle_t *cycle) {
	int value;
	hky_uint_t i;
	hky_listening_t *ls;

#if (HKY_HAVE_DEFERRED_ACCEPT&&defined SO_ACCEPTFILTER)
	struct accept_filter_arg af;
#endif // (HKY_HAVE_DEFERRED_ACCEPT&&defined SO_ACCEPTFILTER)
	ls = cycle->listening.elts;
	for (i = 0; i < cycle->listening.nelts; i++) {
		ls[i].log = *ls[i].logp;
		if (ls[i].rcvbuf != -1) {
			if (setsockopt(ls[i].fd, SOL_SOCKET, SO_RCVBUF,
				(const void *)&ls[i].rcvbuf, sizeof(int))
				== -1) {
				hky_log_error(HKY_LOG_ALERT, cycle->log, hky_socket_errno,
					"setsockopt(SO_RCVBUF,%d) %V failed,ignored",
					ls[i].rcvbuf, &ls[i].addr_text);
			}
		}
		if (ls[i].sndbuf != -1) {
			if (setsockopt(ls[i].fd, SOL_SOCKET, SO_SNDBUF,
				(const void *)&ls[i].sndbuf, sizeof(int))
				== -1) {
				hky_log_error(HKY_LOG_ALERT, cycle->log, hky_socket_errno,
					"setsockopt(SO_SNDBUF,%d) %V failed,ignored",
					ls[i].sndbuf, &ls[i].addr_text);
			}
		}

		if (ls[i].keepalive) {
			value = (ls[i].keepalive == 1) ? 1 : 0;
			if (setsockopt(ls[i].fd, SOL_SOCKET, SO_KEEPALIVE,
				(const void *)&value, sizeof(int))
				== -1) {
				hky_log_error(HKY_LOG_ALERT, cycle->log, hky_socket_errno,
					"setsockopt(SO_KEEPALIVE,%d) %V failed,ignored",
					value, &ls[i].addr_text);
			}
		}
#if (HKY_HAVE_KEEPALIVE_TUNABLE)
		if (ls[i].keepidle) {
			value = ls[i].keepidle;
#if (HKY_KEEPALIVE_FACTOR)
			value *= HKY_KEEPALIVE_FACTOR;
#endif // (HKY_KEEPALIVE_FACTOR)
			if (setsockopt(ls[i].fd, IPPROTO_TCP, TCP_KEEPIDLE,
				(const void *)&value, sizeof(int))
				== -1) {
				hky_log_error(HKY_LOG_ALERT, cycle->log, hky_socket_errno,
					"setsockopt(TCP_KEEPIDLE,%d) %V failed,ignored",
					value, &ls[i].addr_text);
			}
		}
		if (ls[i].keepintvl) {
			value = ls[i].keepintvl;
#if (HKY_KEEPALIVE_FACTOR)
			value *= HKY_KEEPALIVE_FACTOR;
#endif // (HKY_KEEPALIVE_FACTOR)
			if (setsockopt(ls[i].fd, IPPROTO_TCP, TCP_KEEPINTVL,
				(const void *)&value, sizeof(int))
				== -1) {
				hky_log_error(HKY_LOG_ALERT, cycle->log, hky_socket_errno,
					"setsockopt(TCP_KEEPINTVL,%d) %V failed,ignored",
					value, &ls[i].addr_text);
			}
		}
		if (ls[i].keepcnt) {
			if (setsockopt(ls[i].fd, IPPROTO_TCP, TCP_KEEPCNT,
				(const void *)&ls[i].keepcnt, sizeof(int))
				== -1) {
				hky_log_error(HKY_LOG_ALERT, cycle->log, hky_socket_errno,
					"setsockopt(TCP_KEEPCNT, %d) %V failed,ignored",
					ls[i].keepcnt, &ls[i].addr_text);
			}
		}
#endif // (HKY_HAVE_KEEPALIVE_TUNABLE)

#if (HKY_HAVE_SETFIB)
		if (ls[i].setfib != -1) {
			if (setsockopt(ls[i].fd, SOL_SOCKET, SO_SETFIB,
				(const void *)&ls[i].setfib, sizeof(int))
				== -1) {
				hky_log_error(HKY_LOG_ALERT, cycle->log, hky_socket_errno,
					"setsockopt(SO_SETFIB,%d) %V failed,ignored",
					ls[i].setfib, &ls[i].addr_text);
			}
		}
#endif // (HKY_HAVE_SETFIB)

#if (HKY_HAVE_TCP_FASTOPEN)
		if (ls[i].fastopen != -1) {
			if (setsockopt(ls[i].fd, IPPROTO_TCP, TCP_FASTOPEN,
				(const void *)&ls[i].fastopen, sizeof(int))
				== -1) {
				hky_log_error(HKY_LOG_ALERT, cycle->log, hky_socket_errno,
					"setsockopt(TCP_FASTOPEN,%d) %V failed,ignored",
					ls[i].fastopen, &ls[i].addr_text);
			}
		}
#endif // (HKY_HAVE_TCP_FASTOPEN)

#if 0
		if (1) {
			int tcp_nodelay = 1;
			if (setsockopt(ls[i].fd, IPPROTO_TCP, TCP_NODELAY,
				(const void *)&tcp_nodelay, sizeof(int))
				== -1) {
				hky_log_error(HKY_LOG_ALERT, cycle->log, hky_socket_errno,
					"setsockopt(TCP_NODELAY) %V failed,ignored",
					&ls[i].addr_text);
			}
		}
#endif // 0
		if (ls[i].listen) {
			if (listen(ls[i].fd, ls[i].backlog) == -1) {
				hky_log_error(HKY_LOG_ALERT, cycle->log, hky_socket_errno,
					"listen() to %V,backlog %d failed,ignored",
					&ls[i].addr_text, ls[i].backlog);
			}
		}
#if (HKY_HAVE_DEFERRED_ACCEPT)
#ifdef SO_ACCEPTFILTER
		if (ls[i].delete_deferred) {
			if (setsockopt(ls[i].fd, SOL_SOCKET, SO_ACCEPTFILTER, NULL, 0)
				== -1) {
				hky_log_error(HKY_LOG_ALERT, cycle->log, hky_socket_errno,
					"setsockopt(SO_ACCEPTFILTER,NULL) "
					"for %V failed,ignored",
					&ls[i].addr_text);
				if (ls[i].accept_filter) {
					hky_log_error(HKY_LOG_ALERT, cycle->log, 0
						"could not change the accept filter "
						"to \"%s\" for %V,ignored",
						ls[i].accept_filter, &ls[i].addr_text);
				}
				continue;
			}
			ls[i].deferred_accept = 0;
		}
		if (ls[i].add_deferred) {
			hky_memzero(&af, sizeof(struct accept_filter_arg));
			(void)hky_cpystrn((hky_uchar*)af.af_name,
				(hky_uchar*)ls[i].accept_filter, 16);
			if (setsockopt(ls[i].fd, SOL_SOCKET, SO_ACCEPTFILTER,
				&af, sizeof(struct accept_filter_arg))
				== -1) {
				hky_log_error(HKY_LOG_ALERT, cycle->log, hky_socket_errno,
					"setsockopt(SO_ACCEPTFILTER,\"%s\") "
					"for %V failed,ignored",
					ls[i].accept_filter, &ls[i].addr_text);
				continue;
			}
			ls[i].deferred_accept = 1;
		}
#endif // SO_ACCEPTFILTER
#ifdef TCP_DEFER_ACCEPT
		if (ls[i].add_deferred || ls[i].delete_deferred) {
			if (ls[i].add_deferred) {
				value = 1;
			}
			else {
				value = 0;
			}
			if (setsockopt(ls[i].fd, IPPROOTO_TCP, TCP_DEFER_ACCEPT,
				&value, sizeof(int))
				== -1) {
				hky_log_error(HKY_LOG_ALERT, cycle->log, hky_socket_errno,
					"setsockopt(TCP_DEFER_ACCEPT,%d) for %V failed, "
					"ignored",
					value, &ls[i].addr_text);
				continue;
			}
		}
		if (ls[i].add_deferred) {
			ls[i].deferred_accept = 1;
		}
#endif // TCP_DEFER_ACCEPT
#endif // (HKY_HAVE_DEFERRED_ACCEPT)

#if (HKY_HAVE_IP_RECVDSTADDR)
		if (ls[i].wildcard
			&&ls[i].type == SOCK_DGRAM
			&& ls[i].sockaddr->sa_family == AF_INET) {
			value = 1;
			if (setsockopt(ls[i].fd, IPPROTO_IP, IP_RECVDSTADDR,
				(const void *)&value, sizeof(int))
				== -1) {
				hky_log_error(HKY_LOG_ALERT, cycle->log, hky_socket_errno,
					"setsockopt(IP_RECVDSTADDR) "
					"for %V failed,ignored",
					&ls[i].addr_text);
			}
		}
#elif (HKY_HAVE_IP_PKTINFO)
		if (ls[i].wildcard
			&&ls[i].type == SOCK_DGRAM
			&& ls[i].sockaddr->sa_family == AF_INET) {
			value = 1;
			if (setsockopt(ls[i].fd, IPPROTO_IP, IP_PKTINFO,
				(const void *)&value, sizeof(int))
				== -1) {
				hky_log_error(HKY_LOG_ALERT, cycle->log, hky_socket_errno,
					"setsockopt(IP_PKTINFO) "
					"for %V failed,ignored",
					&ls[i].addr_text);
			}
		}
#endif // (HKY_HAVE_IP_RECVDSTADDR)
#if (HKY_HAVE_INET6&&HKY_HAVE_IPV6_RECVPKTINFO)
		if (ls[i].wildcard
			&&ls[i].type == SOCK_DGRAM
			&& ls[i].sockaddr->sa_family == AF_INET6) {
			value = 1;
			if (setsockopt(ls[i].fd, IPPROTO_IPV6, IPV6_RECVPKTINFO,
				(const void *)&value, sizeof(int))
				== -1){
				hky_log_error(HKY_LOG_ALERT, cycle->log, hky_socket_errno,
					"setsockopt(IPV6_RECVPKTINFO) "
					"for %V failed,ignored",
					&ls[i].addr_text);
			}
		}
#endif // (HKY_HAVE_INET6&&HKY_HAVE_IPV6_RECVPKTINFO)
	}
	return;
}