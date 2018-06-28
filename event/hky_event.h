#ifndef _HKY_EVENT_H_INCLUDED_
#define	_HKY_EVENT_H_INCLUDED_

#include "../core/hky_config.h"
#include "../core/hky_core.h"

extern hky_os_io_t hky_io;

#define hky_recv			hky_io.recv
#define hky_recv_chain		hky_io.recv_chain
#define	hky_udp_recv		hky_io.udp_recv
#define	hky_send			hky_io.send
#define	hky_send_chain		hky_io.send_chain
#define hky_udp_send		hky_io.udp_send
#define	hky_udp_send_chain	hky_io.udp_send_chain

#define HKY_USE_IOCP_EVENT	0x00000200

#define hky_add_timer	hky_event_add_timer
#define hky_del_timer	hky_event_del_timer

struct hky_event_s {
	void	*data;

	unsigned	timer_set : 1;

	unsigned	ready : 1;

	hky_event_handler_pt	handler;

	hky_rbtree_node_t timer;

	hky_log_t	*log;
};

extern hky_uint_t hky_event_flags;

#define hky_event_ident(e)	((hky_connection_t *)(p))->fd

#endif // !_HKY_EVENT_H_INCLUDED_
