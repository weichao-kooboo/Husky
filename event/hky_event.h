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

#define	HKY_USE_LEVEL_EVENT		0x00000001

#define	HKY_USE_ONESHOT_EVENT	0x00000002

#define HKY_USE_CLEAR_EVENT		0x00000004

#define HKY_USE_KQUEUE_EVENT	0x00000008

#define HKY_USE_LOWAT_EVENT		0x00000010

#define HKY_USE_GREEDY_EVENT	0x00000020

#define HKY_USE_EPOLL_EVENT		0x00000040

#define HKY_USE_RTSIG_EVENT		0x00000080

#define HKY_USE_AIO_EVENT		0x00000100

#define HKY_USE_IOCP_EVENT		0x00000200

#define HKY_USE_FD_EVENT		0x00000400

#define HKY_USE_TIMER_EVENT		0x00000800

#define HKY_USE_EVENTPORT_EVENT	0x00001000

#define HKY_USE_VNODE_EVENT		0x00002000

#define HKY_CLOSE_EVENT		1

#define HKY_DISABLE_EVENT	2

#define HKY_FLUSH_EVENT		4

#define HKY_LOWAT_EVENT		0
#define HKY_VNODE_EVENT		0

#define hky_add_timer	hky_event_add_timer
#define hky_del_timer	hky_event_del_timer

struct hky_event_s {
	void	*data;

	unsigned write : 1;
	unsigned accept : 1;

	unsigned instance : 1;

	unsigned active : 1;

	unsigned disabled : 1;

	unsigned ready : 1;

	unsigned oneshot : 1;

	unsigned complete : 1;

	unsigned eof : 1;
	unsigned error : 1;

	unsigned timedout : 1;
	unsigned timer_set : 1;

	unsigned delayed : 1;
	unsigned deferred_accept : 1;

	unsigned pending_eof : 1;

	unsigned posted : 1;
	unsigned closed : 1;
	unsigned channel : 1;
	unsigned resolver : 1;
	unsigned cancelable : 1;
#if (HKY_HAVE_KQUEUE)
	unsigned kq_vnode : 1;
	int kq_errno;
#endif // (HKY_HAVE_KQUEUE)

#if (HKY_HAVE_KQUEUE)||(HKY_HAVE_IOCP)
	int available;
#else
	unsigned available : 1;
#endif // (HKY_HAVE_KQUEUE)||(HKY_HAVE_IOCP)

	hky_event_handler_pt	handler;

#if (HKY_HAVE_IOCP)
	hky_event_ovlp_t ovlp;
#endif // (HKY_HAVE_IOCP)
	hky_uint_t index;

	hky_log_t *log;

	hky_queue_t queue;

	hky_rbtree_node_t timer;

#if 0
	void *thr_ctx;
#if(HKY_EVENT_T_PADDING)
	hky_uint32 padding[HKY_EVENT_T_PADDING];
#endif
#endif // 0

};

extern hky_uint_t hky_event_flags;
#if (HKY_HAVE_EPOLLRDHUP)
extern hky_uint_t	hky_use_epoll_rdhup;
#endif // (HKY_HAVE_EPOLLRDHUP)


#define hky_event_ident(e)	((hky_connection_t *)(p))->fd

#endif // !_HKY_EVENT_H_INCLUDED_
