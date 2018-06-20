
#include "hky_config.h"
#include "hky_core.h"

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