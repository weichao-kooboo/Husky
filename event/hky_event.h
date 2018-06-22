#ifndef _HKY_EVENT_H_INCLUDED_
#define	_HKY_EVENT_H_INCLUDED_

#include "../core/hky_config.h"
#include "../core/hky_core.h"

#define HKY_USE_IOCP_EVENT	0x00000200

#define hky_add_timer	hky_event_add_timer
#define hky_del_timer	hky_event_del_timer

struct hky_event_s {
	void	*data;

	unsigned	timer_set : 1;

	hky_event_handler_pt	handler;

	hky_rbtree_node_t timer;

	hky_log_t	*log;
};

extern hky_uint_t hky_event_flags;

#define hky_event_ident(e)	((hky_connection_t *)(p))->fd

#endif // !_HKY_EVENT_H_INCLUDED_
