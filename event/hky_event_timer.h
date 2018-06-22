#pragma once

#ifndef _HKY_EVENT_TIMER_H_INCLUDED_
#define _HKY_EVENT_TIMER_H_INCLUDED_

#include "../core/hky_config.h"
#include "../core/hky_core.h"
#include "hky_event.h"

#define HKY_TIMER_LAZY_DELAY	300

extern hky_rbtree_t hky_event_timer_rbtree;

static hky_inline void
hky_event_del_timer(hky_event_t *ev) {
	hky_log_debug2(HKY_LOG_DEBUG_EVENT, ev->log, 0,
		"event timer del: %d: %M",
		hky_event_ident(ev->data), ev->timer.key);

	hky_rbtree_delete(&hky_event_timer_rbtree, &ev->timer);

#if (HKY_DEBUG)
	ev->timer.left = NULL;
	ev->timer.right = NULL;
	ev->timer.parent = NULL;
#endif // (HKY_DEBUG)
	ev->timer_set = 0;
}

static hky_inline void
hky_event_add_timer(hky_event_t *ev, hky_msec_t timer) {
	hky_msec_t key;
	hky_msec_int_t diff;

	key = hky_current_msec + timer;
	if (ev->timer_set) {
		diff = (hky_msec_int_t)(key - ev->timer.key);
		if (hky_abs(diff) < HKY_TIMER_LAZY_DELAY) {
			hky_log_debug3(HKY_LOG_DEBUG_EVENT, ev->log, 0,
				"event timer: %d, old:%M, new: %M",
				hky_event_ident(ev->data), ev->timer.key, key);
			return;
		}
		hky_del_timer(ev);
	}
	ev->timer.key = key;
	hky_log_debug3(HKY_LOG_DEBUG_EVENT, ev->log, 0,
		"event timer add: %d: %M:%M",
		hky_event_ident(ev->data), timer, ev->timer.key);
	hky_rbtree_insert(&hky_event_timer_rbtree, &ev->timer);
	ev->timer_set = 1;
}

#endif