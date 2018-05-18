#ifndef HKY_CONNECTION_H_INCLUDED
#define HKY_CONNECTION_H_INCLUDED

#include "hky_config.h"
#include "hky_core.h"

typedef struct hky_listening_s hky_listening_t;

struct hky_listening_s {
	//监听套接字的文件描述符
	hky_socket_t fd;
	//监听套接字地址
	struct sockaddr *sockaddr;
	//监听套接字地址长度
	socklen_t socklen;
	//套接字长度范围的最大值
	size_t addr_text_max_len;
	//字符串表示套接字的详细信息
	hky_str_t addr_text;
	//获取套接字的类型和状态
	int type;
	//获取套接字的接收缓冲区
	int rcvbuf;
	//获取套接字的发送缓冲区
	int sndbuf;
	//监听套接字绑定的最大队列
	int backlog;
#if(HKY_HAVE_DEFERRED_ACCEPT && defined SO_ACCEPTFILTER)
	char *accept_filter;
#endif
#if(HKY_HAVE_SETFIB)
	int setfib;
#endif

	unsigned ignore : 1;
#if(HKY_HAVE_TCP_FASTOPEN)
	//获取套接字类型是fastopen
	int fastopen;
#endif
};

struct hky_connection_s{

};

hky_int_t hky_set_inherited_sockets(hky_cycle_t *cycle);

#endif // HKY_CONNECTION_H_INCLUDED
