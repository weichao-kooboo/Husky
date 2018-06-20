#ifndef HKY_SOCKET_H_INCLUDED
#define HKY_SOCKET_H_INCLUDED

#include "../core/hky_config.h"

typedef int hky_socket_t;

#define hky_socket	socket
#define hky_socket_n	"socket()"

#if (HKY_HAVE_FIONBIO)

int hky_nonblocking(hky_socket_t s);
int hky_blocking(hky_socket_t s);

#define hky_nonblocking_n	"ioctl(FIONBIO)"
#define hky_blocking_n		"ioctl(!FIONBIO)"
#else

#define hky_nonblocking(s) fcntl(s,F_SETFL,fcntl(s,F_GETFL)|O_NONBLOCK)
#define hky_nonblocking_n	"fcntl(O_NONBLOCK)"

#define hky_blocking(s)	fcntl(s,F_SETFL,fcntl(s,F_GETFL)&~O_NONBLOCK)
#define hky_blocking_n	"fcntl(!O_NONBLOCK)"

#endif // (HKY_HAVE_FIONBIO)

#define hky_shutdown_socket shutdown
#define hky_shutdown_socket_n "shutdown()"

#define hky_close_socket	close
#define hky_close_socket_n	"close() socket"

#endif // !HKY_SOCKET_H_INCLUDED