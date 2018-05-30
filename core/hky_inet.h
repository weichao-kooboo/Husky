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

#if(HKY_HAVE_INET6)
hky_int_t hky_inet6_addr(hky_uchar *p, size_t len, hky_uchar *addr);
#endif
size_t hky_sock_ntop(struct sockaddr *sa, socklen_t socklen, hky_uchar *text,
	size_t len, hky_uint_t port);

#endif // HKY_INET_H_INCLUDED
