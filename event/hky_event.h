#ifndef _HKY_EVENT_H_INCLUDED_
#define	_HKY_EVENT_H_INCLUDED_

#include "../core/hky_config.h"
#include "../core/hky_core.h"

#define HKY_USE_IOCP_EVENT	0x00000200

struct hky_event_s {
	void	*data;

	unsigned	timer_set : 1;

	hky_event_handler_pt	handler;

	hky_log_t	*log;
};

extern hky_uint_t hky_event_flags;

#endif // !_HKY_EVENT_H_INCLUDED_
