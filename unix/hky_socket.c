
#include "../core/hky_config.h"
#include "../core/hky_core.h"

#if (HKY_HAVE_FIONBIO)
int 
hky_nonblocking(hky_socket_t s) {
	int nb;
	nb = 1;
	return ioctl(s, FIONBIO, &nb);
}
int
hky_blocking(hky_socket_t s) {
	int nb;
	nb = 0;
	return ioctl(s, FIONBIO, &nb);
}

#endif // (HKY_HAVE_FIONBIO)
