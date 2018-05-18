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
#if(HKY_HAVE_DEFERRED_ACCEPT && defined SO_ACCEPTFILTER)
	char *accept_filter;
#endif
#if(HKY_HAVE_SETFIB)
	int setfib;
#endif

	unsigned ignore : 1;
#if(HKY_HAVE_TCP_FASTOPEN)
	//��ȡ�׽���������fastopen
	int fastopen;
#endif
};

struct hky_connection_s{

};

hky_int_t hky_set_inherited_sockets(hky_cycle_t *cycle);

#endif // HKY_CONNECTION_H_INCLUDED
