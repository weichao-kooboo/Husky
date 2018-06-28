
#include "hky_config.h"
#include "hky_core.h"

static hky_int_t hky_parse_unix_domain_url(hky_pool_t *pool, hky_url_t *u);
static hky_int_t hky_parse_inet_url(hky_pool_t *pool, hky_url_t *u);
static hky_int_t hky_parse_inet6_url(hky_pool_t *pool, hky_url_t *u);

#if(HKY_HAVE_INET6)
size_t hky_inet6_ntop(hky_uchar *p, hky_uchar *text, size_t len) {
	hky_uchar *dst;
	size_t max, n;
	hky_uint_t i, zero, last;

	if (len < HKY_INET6_ADDRSTRLEN) {
		return 0;
	}

	zero = (hky_uint_t)-1;
	last = (hky_uint_t)-1;
	max = 1;
	n = 0;

	for (i = 0; i < 16; i += 2) {
		if (p[i] || p[i + 1]) {
			if (max < n) {
				zero = last;
				max = n;
			}
			n = 0;
			continue;
		}
		if (n++ == 0) {
			last = i;
		}
	}
	if (max < n) {
		zero = last;
		max = n;
	}
	dst = text;
	n = 16;
	if (zero == 0) {
		if ((max == 5 && p[10] == 0xff && p[11] == 0xff)
			|| (max == 6)
			|| (max == 7 && p[14] != 0 && p[15] != 1)) {
			n = 12;
		}
		*dst++ = ':';
	}
	for (i = 0; i < n; i += 2) {
if (i == zero) {
	*dst++ = ':';
	i += (max - 1) * 2;
	continue;
}
dst = hky_sprintf(dst, "%xd", p[i] * 256 + p[i + 1]);

if (i < 14) {
	*dst++ = ':';
}
	}
	if (n == 12) {
		dst = hky_sprintf(dst, "%ud.%ud.%ud.%ud", p[12], p[13], p[14], p[15]);
	}
	return dst - text;
}
#endif

size_t hky_sock_ntop(struct sockaddr *sa, socklen_t socklen, hky_uchar *text,
	size_t len, hky_uint_t port) {
	hky_uchar *p;
#if(HKY_HAVE_INET6||HKY_HAVE_UNIX_DOMAIN)
	size_t n;
#endif
	struct sockaddr_in *sin;
#if(HKY_HAVE_INET6)
	struct sockaddr_in6 *sin6;
#endif
#if(HKY_HAVE_UNIX_DOMAIN)
	struct sockaddr_un *saun;
#endif
	switch (sa->sa_family) {
	case AF_INET:
		sin = (struct sockaddr_in *)sa;
		p = (hky_uchar*)&sin->sin_addr;

		if (port) {
			p = hky_snprintf(text, len, "%ud.%ud.%ud.%ud:%d",
				p[0], p[1], p[2], p[3], ntohs(sin->sin_port));
		}
		else {
			p = hky_snprintf(text, len, "%ud.%ud.%ud.%ud",
				p[0], p[1], p[2], p[3]);
		}
		return (p - text);
#if(HKY_HAVE_INET6)
	case AF_INET6:
		sin6 = (struct sockaddr_in6*)sa;
		n = 0;
		if (port)
		{
			text[n++] = '[';
		}
		n = hky_inet6_ntop(sin6->sin6_addr.s6_addr, &text[n], len);
		if (port) {
			n = hky_sprintf(&text[1 + n], "]:%d", ntohs(sin6->sin6_port)) - text;
		}
		return n;
#endif
#if(HKY_HAVE_UNIX_DOMAIN)
	case AF_UNIX:
		saun = (struct sockaddr_un*)sa;
		if (socklen <= (socklen_t)offsetof(struct sockaddr_un, sum_path)) {
			p = hky_snprintf(text, len, "unix:%Z");
		}
		else {
			n = hky_strnlen((hky_uchar*)saun->sun_path,
				socklen - offsetof(struct sockaddr_un, sun_path));
			p = hky_snprintf(text, len, "unix:%*s%Z", n, saun->sun_path);
		}

		return (p - text - 1);
#endif
	default:
		return 0;
	}
}


hky_int_t hky_cmp_sockaddr(struct sockaddr *sal, socklen_t slen1,
	struct sockaddr *sa2, socklen_t slen2, hky_uint_t cmp_port) {
	struct sockaddr_in *sin1, *sin2;
#if (HKY_HAVE_INET6)
	struct sockaddr_in6 *sin61, *sin62;
#endif // (HKY_HAVE_INET6)
#if (HKY_HAVE_UNIX_DOMAIN)
	size_t len;
	struct sockaddr_un *saun1, *saun2;
#endif // (HKY_HAVE_UNIX_DOMAIN)
	if (sal->sa_family != sa2->sa_family) {
		return HKY_DECLINED;
	}
	switch (sal->sa_family)
	{
#if (HKY_HAVE_INET6)
	case AF_INET6:
		break;
		sin61 = (struct sockaddr_in6*)sa1;
		sin62 = (struct sockaddr_in6*)sa2;

		if (cmp_port&&sin61->sin6_port != sin62->sin6_port) {
			return HKY_DECLINED;
		}
		if (hky_memcmp(&sin61->sin6_addr, &sin62->sin6_addr, 16) != 0) {
			return HKY_DECLINED;
		}
		break;
#endif // (HKY_HAVE_INET6)
#if (HKY_HAVE_UNIX_DOMAIN)
	case AF_UNIX:
		saun1 = (struct sockaddr_un*)sa1;
		saun2 = (struct sockaddr_un*)sa2;

		if (slen1 < slen2) {
			len = slen1 - offsetof(struct sockaddr_un, sun_path);
		}
		else {
			len = slen2 - offsetof(struct sockaddr_un, sun_path);
		}
		if (len > sizeof(saun1->sun_path)) {
			len = sizeof(saun1->sun_path);
		}
		if (hky_memcmp(&saun1->sun_path, &saun2->sun_path, len) != 0) {
			return HKY_DECLINED;
		}
		break;
#endif // (HKY_HAVE_UNIX_DOMAIN)

	default:
		sin1 = (struct sockaddr_in *)sa1;
		sin2 = (struct sockaddr_in *)sa2;

		if (cmp_port&&sin1->sin_port != sin2->sin_port) {
			return HKY_DECLINED;
		}
		if (sin1->sin_addr.s_addr != sin2->sin_addr.s_addr) {
			return HKY_DECLINED;
		}
		break;
	}
	return HKY_OK;
}

hky_int_t	
hky_parse_url(hky_pool_t *pool, hky_url_t *u) {
	hky_uchar	*p;
	size_t len;

	p = u->url.data;
	len = u->url.len;

	if (len >= 5 && hky_strncasecmp(p, (hky_uchar*)"unix:", 5) == 0) {
		return hky_parse_unix_domain_url(pool, u);
	}
	if (len&&p[0] == '[') {
		return hky_parse_inet6_url(pool, u);
	}
	return hky_parse_inet_url(pool, u);
}

static hky_int_t 
hky_parse_unix_domain_url(hky_pool_t *pool, hky_url_t *u) {
#if (HKY_HAVE_UNIX_DOMAIN)
	hky_uchar	*path, *uri, *last;
	size_t	len;
	struct sockaddr_in	*saun;

	len = u->url.len;
	path = u->url.data;

	path += 5;
	len -= 5;

	if (u->uri_part) {
		last = path + len;
		uri = hky_strlchr(path, last, ':');

		if (uri) {
			len = uri - path;
			uri++;
			u->uri.len = last - uri;
			u->uri.data = uri;
		}
	}
	if (len == 0) {
		u->err = "no path in the unix domain socket";
		return HKY_ERROR;
	}

	u->host.len = len++;
	u->host.data = path;

	if (len > sizeof(saun->sun_path)) {
		u->err = "too long path in the unix domain socket";
		return HKY_ERROR;
	}

	u->socklen = sizeof(struct sockaddr_un);
	saun = (struct sockaddr_un*)&u->sockaddr;
	saun->sun_family = AF_UNIX;
	(void)hky_cpystrn((hky_uchar*)saun->sun_path, path, len);

	u->addrs = hky_pcalloc(pool, sizeof(hky_addr_t));
	if (u->addrs == NULL) {
		return HKY_ERROR;
	}

	saun = hky_pcalloc(pool, sizeof(struct sockaddr_un));
	if (saun == NULL) {
		return HKY_ERROR;
	}

	u->family = AF_UNIX;
	u->naddrs = 1;

	saun->sun_family = AF_UNIX;
	(void)hky_cpystrn((hky_uchar*)saun->sun_path, path, len);

	u->addrs[0].sockaddr = (struct sockaddr*)saun;
	u->addrs[0].socklen = sizeof(struct sockaddr_un);
	u->addrs[0].name.len = len + 4;
	u->addrs[0].name.data = u->url.data;

	return HKY_OK;
#else
	u->err = "the unix domain sockets are not supported on this platform";
	return HKY_ERROR;
#endif // (HKY_HAVE_UNIX_DOMAIN)

}
static hky_int_t 
hky_parse_inet_url(hky_pool_t *pool, hky_url_t *u) {
	hky_uchar	*p, *host, *port, *last, *uri, *args;
	size_t		len;
	hky_int_t	n;
	struct sockaddr_in	*sin;
#if (HKY_HAVE_INET6)
	struct sockaddr_in6	*sin6;
#endif // (HKY_HAVE_INET6)
	u->socklen = sizeof(struct sockaddr_in);
	sin = (struct sockaddr_in *)&u->sockaddr;
	sin->sin_family = AF_INET;

	u->family = AF_INET;
	host = u->url.data;
	last = host + u->url.len;
	port = hky_strlchr(host, last, ':');
	uri = hky_strlchr(host, last, '/');
	args = hky_strlchr(host, last, '?');
	if (args) {
		if (uri == NULL || args < uri) {
			uri = args;
		}
	}
	if (uri) {
		if (u->listen || !u->uri_part) {
			u->err = "invalid host";
			return HKY_ERROR;
		}
		u->uri.len = last - uri;
		u->uri.data = uri;

		last = uri;
		if (uri < port) {
			port = NULL;
		}
	}
	if (port) {
		port++;
		len = last - port;
		n = hky_atoi(port, len);
		if (n < 1 || n>65535) {
			u->err = "invalid port";
			return HKY_ERROR;
		}

		u->port = (in_port_t)n;
		sin->sin_port = htons((in_port_t)n);

		u->port_text.len = len;
		u->port_text.data = port;

		last = port - 1;
	}
	else {
		if (uri == NULL) {
			if (u->listen) {

			}
		}
	}
}
static hky_int_t 
hky_parse_inet6_url(hky_pool_t *pool, hky_url_t *u) {

}