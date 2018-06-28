#ifndef HKY_INET_H_INCLUDED
#define HKY_INET_H_INCLUDED

#include "hky_config.h"
#include "hky_core.h"

#define HKY_INET_ADDRSTRLEN		(sizeof("255.255.255.255")-1)
#define HKY_INET6_ADDRSTRLEN												\
		(sizeof("ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255")-1)
#define	HKY_UNIX_ADDRSTRLEN													\
		(sizeof("unix:") -1 +												\
		sizeof(struct sockaddr_un)-offsetof(struct sockaddr_un,sun_path))

typedef union {
	struct sockaddr sockaddr;
	struct sockaddr_in sockaddr_in;
#if(HKY_HAVE_INET6)
	struct sockaddr_in6 sockaddr_in6;
#endif
#if(HKY_HAVE_UNIX_DOMAIN)
	struct sockaddr_un sockaddr_un;
#endif
} hky_sockaddr_t;

typedef struct {
        struct sockaddr *sockaddr;
        socklen_t socklen;
        hky_str_t name;
}   hky_addr_t;

typedef	struct {
	hky_str_t	url;
	hky_str_t	host;
	hky_str_t	port_text;
	hky_str_t	uri;

	in_port_t	port;
	in_port_t	default_port;
	int			family;

	unsigned	listen : 1;
	unsigned	uri_part : 1;
	unsigned	no_resolve : 1;

	unsigned	no_port : 1;
	unsigned	wildcard : 1;
	
	socklen_t	socklen;
	hky_sockaddr_t	sockaddr;

	hky_addr_t	*addrs;
	hky_uint_t	naddrs;

	char		*err;
}hky_url_t;

#if(HKY_HAVE_INET6)
hky_int_t hky_inet6_addr(hky_uchar *p, size_t len, hky_uchar *addr);
#endif
size_t hky_sock_ntop(struct sockaddr *sa, socklen_t socklen, hky_uchar *text,
	size_t len, hky_uint_t port);
hky_int_t hky_cmp_sockaddr(struct sockaddr *sal, socklen_t slen1,
	struct sockaddr *sa2, socklen_t slen2, hky_uint_t cmp_port);
hky_int_t	hky_parse_url(hky_pool_t *pool, hky_url_t *u);

#endif // HKY_INET_H_INCLUDED
