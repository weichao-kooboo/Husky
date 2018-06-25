#ifndef HKY_CONNECTION_H_INCLUDED
#define HKY_CONNECTION_H_INCLUDED

#include "hky_config.h"
#include "hky_core.h"

typedef struct hky_listening_s hky_listening_t;

struct hky_listening_s {
	//�����׽��ֵ��ļ�������
	hky_socket_t fd;
	//�����׽��ֵ�ַ
	struct sockaddr *sockaddr;
	//�����׽��ֵ�ַ����
	socklen_t socklen;
	//�׽��ֳ��ȷ�Χ�����ֵ
	size_t addr_text_max_len;
	//�ַ�����ʾ�׽��ֵ���ϸ��Ϣ
	hky_str_t addr_text;
	//��ȡ�׽��ֵ����ͺ�״̬
	int type;
	//��ȡ�׽��ֵĽ��ջ�����
	int rcvbuf;
	//��ȡ�׽��ֵķ��ͻ�����
	int sndbuf;
	//�����׽��ְ󶨵�������
	int backlog;
#if (HKY_HAVE_KEEPALIVE_TUNABLE)
	int keepidle;
	int keepintvl;
	int keepcnt;
#endif // (HKY_HAVE_KEEPALIVE_TUNABLE)

#if(HKY_HAVE_DEFERRED_ACCEPT && defined SO_ACCEPTFILTER)
	char *accept_filter;
#endif
#if(HKY_HAVE_SETFIB)
	int setfib;
#endif
	hky_log_t log;
	hky_log_t *logp;

	hky_listening_t		*previous;
	hky_connection_t	*connection;

	unsigned open : 1;
	unsigned remain : 1;
	unsigned ignore : 1;

	unsigned bound : 1;
	unsigned inherited : 1;
	unsigned nonblocking_accept : 1;
	unsigned listen : 1;
	unsigned shared : 1;
	unsigned addr_ntop : 1;
	unsigned wildcard : 1;

	unsigned reuseport : 1;
	unsigned add_reuseport : 1;
	unsigned keepalive : 2;

	unsigned deferred_accept : 1;
	unsigned delete_deferred : 1;
	unsigned add_deferred : 1;

#if(HKY_HAVE_TCP_FASTOPEN)
	//��ȡ�׽���������fastopen
	int fastopen;
#endif
};

struct hky_connection_s{
	hky_socket_t	fd;
};

hky_int_t hky_set_inherited_sockets(hky_cycle_t *cycle);
hky_int_t hky_open_listening_sockets(hky_cycle_t *cycle);
void hky_configure_listening_sockets(hky_cycle_t *cycle);

#endif // HKY_CONNECTION_H_INCLUDED
