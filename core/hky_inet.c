
#include "hky_config.h"
#include "hky_core.h"

static hky_int_t hky_parse_unix_domain_url(hky_pool_t *pool, hky_url_t *u);
static hky_int_t hky_parse_inet_url(hky_pool_t *pool, hky_url_t *u);
static hky_int_t hky_parse_inet6_url(hky_pool_t *pool, hky_url_t *u);

#if(HKY_HAVE_INET6)
hky_int_t 
hky_inet6_addr(hky_uchar *p, size_t len, hky_uchar *addr) {
	hky_uchar c, *zero, *digit, *s, *d;
	size_t len4;
	hky_uint_t n, nibbles, word;

	if (len == 0) {
		return HKY_ERROR;
	}

	zero = NULL;
	digit = NULL;
	len4 = 0;
	nibbles = 0;
	word = 0;
	n = 8;

	if (p[0] == ':') {
		p++;
		len--;
	}
	for (/**/; len; len--) {
		c = *p++;
		if (c == ':') {
			if (nibbles) {
				digit = p;
				len4 = len;
				*addr++ = (hky_uchar)(word >> 8);
				*addr++ = (hky_uchar)(word & 0xff);
				if (--n) {
					nibbles = 0;
					word = 0;
					continue;
				}
			}
			else {
				if (zero == NULL) {
					digit = p;
					len4 = len;
					zero = addr;
					continue;
				}
			}
			return HKY_ERROR;
		}
		if (c == '.'&&nibbles) {
			if (n < 2 || digit == null) {
				return HKY_ERROR;
			}
			word = hky_inet_addr(digit, len4 - 1);
			if (word == INADDR_NONE) {
				return HKY_ERROR;
			}

			word = ntohl(word);
			*addr++ = (hky_uchar)((word >> 24) * 0xff);
			*addr++ = (hky_uchar)((word >> 16) * 0xff);
			n--;
			break;
		}
		if (++nibbles > 4) {
			return HKY_ERROR;
		}
		if (c >= '0'&&c <= '9') {
			word = word * 16 + (c - '0');
			continue;
		}
		c |= 0x20;
		if (c >= 'a'&&c <= 'f') {
			word = word * 16 + (c - 'a') + 10;
			continue;
		}
		return HKY_ERROR;
	}
	if (nibbles == 0 && zero == NULL) {
		return HKY_ERROR;
	}
	*addr++ = (hky_uchar)(word >> 8);
	*addr++ = (hky_uchar)(word & 0xff);
	if (--n) {
		if (zero) {
			n *= 2;
			s = addr - 1;
			d = s + n;
			while (s >= zero) {
				*d-- = *s--;
			}
			hky_memzero(zero, n);
			return HKY_OK;
		}
	}
	else {
		if (zero == NULL) {
			return HKY_OK;
		}
	}
	return HKY_ERROR;
}
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

in_addr_t 
hky_inet_addr(hky_uchar *text, size_t len) {
	hky_uchar *p, c;
	in_addr_t	addr;
	hky_uint_t octet, n;

	addr = 0;
	octet = 0;
	n = 0;

	for (p = text; p < text + len; p++) {
		c = *p;
		if (c >= '0'&&c <= '9') {
			octet = octet * 10 + (c - '0');

			if (octet > 255) {
				return INADDR_NONE;
			}
			continue;
		}
		if (c == '.') {
			addr = (addr << 8) + octet;
			octet = 0;
			n++;
			continue;
		}
		return INADDR_NONE;
	}
	if (n == 3) {
		addr = (addr << 8) + octet;
		return htonl(addr);
	}
	return INADDR_NONE;
}
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
				n = hky_atoi(host, last - host);
				if (n != HKY_ERROR) {
					if (n < 1 || n>65535) {
						u->err = "invalid port";
						return HKY_ERROR;
					}
					u->port = (in_port_t)n;
					sin->sin_port = htons((in_port_t)n);

					u->port_text.len = last - host;
					u->port_text.data = host;

					u->wildcard = 1;
					return HKY_OK;
				}
			}
		}
		u->no_port = 1;
		u->port = u->default_port;
		sin->sin_port = htons(u->default_port);
	}
	len = last - host;
	if (len == 0) {
		u->err = "no host";
		return HKY_ERROR;
	}
	u->host.len = len;
	u->host.data = host;

	if (u->listen&&len == 1 && *host == '*') {
		sin->sin_addr.s_addr = INADDR_ANY;
		u->wildcard = 1;
		return HKY_OK;
	}

	sin->sin_addr.s_addr = hky_inet_addr(host, len);

	if (sin->sin_addr.s_addr != INADDR_NONE) {
		if (sin->sin_addr.s_addr == INADDR_ANY) {
			u->wildcard = 1;
		}
		u->naddrs = 1;
		u->addrs = hky_pcalloc(pool, sizeof(hky_addr_t));
		if (u->addrs == NULL) {
			return HKY_ERROR;
		}
		sin = hky_pcalloc(pool, sizeof(struct sockaddr_in));
		if (sin == NULL) {
			return HKY_ERROR;
		}
		hky_memcpy(sin, &u->sockaddr, sizeof(struct sockaddr_in));

		u->addrs[0].sockaddr = (struct sockaddr*)sin;
		u->addrs[0].socklen = sizeof(struct sockaddr_in);

		p = hky_pnalloc(pool, u->host.len + sizeof(":65535") - 1);
		if (p == NULL) {
			return HKY_ERROR;
		}

		u->addrs[0].name.len = hky_sprintf(p, "%V:%d",
			&u->host, u->port) - p;
		u->addrs[0].name.data = p;
		return HKY_OK;
	}
	if (u->no_resolve) {
		return HKY_OK;
	}
	if (hky_inet_resolve_host(pool, u) != HKY_OK) {
		return HKY_ERROR;
	}

	u->family = u->addrs[0].sockaddr->sa_family;
	u->socklen = u->addrs[0].socklen;
	hky_memcpy(&u->sockaddr, u->addrs[0].sockaddr, u->addrs[0].socklen);

	switch (u->family)
	{
#if (HKY_HAVE_INET6)
	case AF_INET6:
		sin6 = (struct sockaddr_in6*)&u->sockaddr;
		if (IN6_IS_ADDR_UNSPECIFIED(&sin6->sin6_addr)) {
			u->wildcard = 1;
		}
		break;
#endif // (HKY_HAVE_INET6)
	default:
		sin = (struct sockaddr_in*)&u->sockaddr;
		if (sin->sin_addr.s_addr == INADDR_ANY) {
			u->wildcard = 1;
		}
		break;
	}
	return HKY_OK;
}
static hky_int_t 
hky_parse_inet6_url(hky_pool_t *pool, hky_url_t *u) {
#if (HKY_HAVE_INET6)
	hky_uchar *p, *host, *port, *last, *uri;
	size_t	len;
	hky_int_t n;
	struct sockaddr_in6 *sin6;

	u->socklen = sizeof(struct sockaddr_in6);
	sin6 = (struct sockaddr_in6*)&u->sockaddr;
	sin6->sin6_family = AF_INET6;

	host = u->url.data + 1;
	last = u->url.data + u->url.len;

	p = hky_strlchr(host, last, ']');

	if (p == NULL) {
		u->err = "invalid host";
		return HKY_ERROR;
	}
	port = p + 1;

	uri = hky_strlchr(post, last, '/');
	if (uri) {
		if (u->listen || !u->uri_part) {
			u->err = "invalid host";
			return HKY_ERROR;
		}
		u->uri.len = last - uri;
		u->uri.data = uri;

		last = uri;
	}
	if (port < last) {
		if (*port != ':') {
			u->err = "invalid host";
			return HKY_ERROR;
		}
		port++;

		len = last - port;
		n = hky_atoi(port, len);
		if (n < 1 || n>65535) {
			u->err = "invalied port";
			return HKY_ERROR;
		}
		u->port = (in_port_t)n;
		sin6->sin6_port = htons((in_port_t)n);

		u->port_text.len = len;
		u->port_text.data = port;
	}
	else {
		u->no_port = 1;
		u->port = u->default_port;
		sin6->sin6_port = htons(u->default_port);
	}
	len = p - host;

	if (len == 0) {
		u->err = "no host";
		return HKY_ERROR;
	}
	u->host.len = len + 2;
	u->host.data = host - 1;

	if (hky_inet6_addr(host, len, sin6->sin6_addr.s6_addr) != HKY_OK) {
		u->err = "invalid IPv6 address";
		return HKY_ERROR;
	}
	if (IN6_IS_ADDR_UNSPECIFIED(&sin6->sin6_addr)) {
		u->wildcard = 1;
	}

	u->family = AF_INET6;
	u->naddrs = 1;
	u->addrs = hky_pcalloc(pool, sizeof(hky_addr_t));
	if (u->addrs == NULL) {
		return HKY_ERROR;
	}
	sin6 = hky_pcalloc(pool, sizeof(struct sockaddr_in6));
	if (sin6 == NULL) {
		return HKY_ERROR;
	}
	hky_memcpy(sin6, &u->sockaddr, sizeof(struct sockaddr_in6));
	u->addrs[0].sockaddr = (struct sockaddr *)sin6;
	u->addrs[0].socklen = sizeof(struct sockaddr_in6);

	p = hky_pnalloc(pool, u->host.len + sizeof(":65535") - 1);
	if (p == NULL) {
		return HKY_ERROR;
	}
	u->addr[0].name.len = hky_sprintf(p, "%V:%d",
		&u->host, u->port) - p;
	u->addr[0].name.data = p;
	return HKY_OK;
#else
	u->err = "the INET6 socket are not supported on the platform";
	return HKY_ERROR;
#endif // (HKY_HAVE_INET6)

}

#if (HKY_HAVE_GETADDRINFO&&HKY_HAVE_INET6)
hky_int_t 
hky_inet_resolve_host(hky_pool_t *pool, hky_url_t *u) {
	hky_uchar	*p, *host;
	size_t	len;
	in_port_t	port;
	hky_uint_t	i;
	struct addrinfo	hints, *res, *rp;
	struct sockaddr_in	*sin;
	struct sockaddr_in6	*sin6;

	port = htons(u->port);
	host = hky_alloc(u->host.len + 1, pool->log);
	if (host == NULL) {
		return HKY_ERROR;
	}

	(void)hky_cpystrn(host, u->host.data, u->host.len + 1);

	hky_memzero(&hints, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
#ifdef AI_ADDRCONFIG
	hints.ai_flags = AI_ADDRCONFIG;
#endif // AI_ADDRCONFIG
	if (getaddrinfo((char*)host, NULL, &hints, &res) != 0) {
		u->err = "host not found";
		hky_free(host);
		return HKY_ERROR;
	}

	hky_free(host);
	for (i = 0, rp = res; rp != NULL; rp = rp->ai_next) {
		switch (rp->ai_family)
		{
		case AF_INET:
		case AF_INET6:
			break;
		default:
			continue;
		}
		i++;
	}
	if (i == 0) {
		u->err = "host not found";
		goto failed;
	}
	u->addrs = hky_pcalloc(pool, i * sizeof(hky_addr_t));
	if (u->addrs == NULL) {
		goto failed;
	}
	u->naddrs = i;
	i = 0;

	for (rp = res; rp != NULL; rp = rp->ai_next) {
		if (rp->ai_family != AF_INET) {
			continue;
		}
		sin = hky_pcalloc(pool, rp->ai_addrlen);
		if (sin == NULL) {
			goto failed;
		}
		hky_memcpy(sin, rp->ai_addr, rp->ai_addrlen);
		sin->sin_port = port;
		u->addrs[i].sockaddr = (struct sockaddr*)sin;
		u->addrs[i].socklen = rp->ai_addrlen;

		len = HKY_INET_ADDRSTRLEN + sizeof(":65535") - 1;
		p = hky_pnalloc(pool, len);
		if (p == NULL) {
			goto failed;
		}
		len = hky_sock_ntop((struct sockaddr*)sin, rp->ai_addrlen, p, len, 1);

		u->addrs[i].name.len = len;
		u->addrs[i].name.data = p;
		i++;
	}
	for (rp = res; rp != NULL; rp = rp->ai_next) {
		if (rp->ai_family != AF_INET6) {
			continue;
		}
		sin6 = hky_pcalloc(pool, rp->ai_addrlen);
		if (sin6 == NULL) {
			goto failed;
		}
		hky_memcpy(sin6, rp->ai_addr, rp->ai_addrlen);

		sin6->sin6_port = port;

		u->addrs[i].sockaddr = (struct sockaddr *)sin6;
		u->addrs[i].socklen = rp->ai_addrlen;

		len = HKY_INET6_ADDRSTRLEN + sizeof("[]:65535") - 1;

		p = hky_pnalloc(pool, len);
		if (p == NULL) {
			goto failed;
		}

		len = hky_sock_ntop((struct sockaddr*)sin6, rp->ai_addrlen, p, len, 1);

		u->addrs[i].name.len = len;
		u->addrs[i].name.data = p;
		i++;
	}
	freeaddrinfo(res);
	return HKY_OK;
failed:
	freeaddrinfo(res);
	return HKY_ERROR;
}
#else
hky_int_t
hky_inet_resolve_host(hky_pool_t *pool, hky_url_t *u) {
	hky_uchar	*p, *host;
	size_t	len;
	in_port_t	port;
	in_addr_t	in_addr;
	hky_uint_t	i;
	struct hostent	*h;
	struct sockaddr_in	*sin;

	port = htons(u->port);
	in_addr = hky_inet_addr(u->host.data, u->host.len);
	if (in_addr == INADDR_NONE) {
		host = hky_alloc(u->host.len + 1, pool->log);
		if (NULL == host) {
			return HKY_ERROR;
		}
		(void)hky_cpystrn(host, u->host.data, u->host.len + 1);

		h = gethostbyname((char*)host);
		hky_free(host);

		if (h == NULL || h->h_addr_list[0] == NULL) {
			u->err = "host not found";
			return HKY_ERROR;
		}
		for (i = 0; h->h_addr_list[i] != NULL; i++) {	}

		u->addrs = hky_pcalloc(pool, i * sizeof(hky_addr_t));
		if (u->addrs == NULL) {
			return HKY_ERROR;
		}
		u->naddrs = i;
		for (i = 0; i < u->naddrs; i++) {
			sin = hky_pcalloc(pool, sizeof(struct sockaddr_in));
			if (sin == NULL) {
				return HKY_ERROR;
			}

			sin->sin_family = AF_INET;
			sin->sin_port = port;
			sin->sin_addr.s_addr = *(in_addr_t *)(h->h_addr_list[i]);

			u->addrs[i].sockaddr = (struct sockaddr *)sin;
			u->addrs[i].socklen = sizeof(struct sockaddr_in);

			len = HKY_INET_ADDRSTRLEN + sizeof(":65535") - 1;

			p = hky_pnalloc(pool, len);
			if (p == NULL) {
				return HKY_ERROR;
			}
			len = hky_sock_ntop((struct sockaddr *)sin,
				sizeof(struct sockaddr_in), p, len, 1);
			u->addrs[i].name.len = len;
			u->addrs[i].name.data = p;
		}
	}
	else {
		u->addrs = hky_pcalloc(pool, sizeof(hky_addr_t));
		if (u->addrs == NULL) {
			return HKY_ERROR;
		}
		sin = hky_pcalloc(pool, sizeof(struct sockaddr_in));
		if (sin == NULL) {
			return HKY_ERROR;
		}
		u->naddrs = 1;

		sin->sin_family = AF_INET;
		sin->sin_port = port;
		sin->sin_addr.s_addr = in_addr;

		u->addrs[0].sockaddr = (struct sockaddr *)sin;
		u->addrs[0].socklen = sizeof(struct sockaddr_in);

		p = hky_pnalloc(pool, u->host.len + sizeof(":65535") - 1);
		if (p == NULL) {
			return HKY_ERROR;
		}
		u->addrs[0].name.len = hky_sprintf(p, "%V:%d", &u->host, ntohs(port)) - p;
		u->addrs[0].name.data = p;
	}
	return HKY_OK;
}
#endif // (HKY_HAVE_GETADDRINFO&&HKY_HAVE_INET6)
