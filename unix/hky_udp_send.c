#pragma once
#include "../core/hky_config.h"
#include "../core/hky_core.h"
#include "../event/hky_event.h"

ssize_t 
hky_udp_unix_send(hky_connection_t *c, hky_uchar *buf, size_t size) {
	ssize_t n;
	hky_err_t err;
	hky_event_t *wev;
	wev = c->write;

	for (;;) {
		n = sendto(c->fd, buf, size, 0, c->sockaddr, c->socklen);
		hky_log_debug4(HKY_LOG_DEBUG_EVENT, c->log, 0,
			"sendto: fd:%d %z of %uz to \"%V\"",
			c->fd, n, size, &c->addr_text);
		if (n >= 0) {
			if ((size_t)n != size) {
				wev->error = 1;
				(void)hky_connection_error(c, 0, "sendto() incomplete");
				return HKY_ERROR;
			}
			c->sent += n;
			return n;
		}
		err = hky_socket_errno;
		if (err == HKY_EAGAIN) {
			wev->ready = 0;
			hky_log_debug0(HKY_LOG_DEBUG_EVENT, c->log, HKY_EAGAIN,
				"sendto() not ready");
			return HKY_AGAIN;
		}
		if (err != HKY_EINTR) {
			wev->error = 1;
			(void)hky_connection_error(c, err, "sendto() failed");
			return HKY_ERROR;
		}
	}
}