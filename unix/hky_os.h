#ifndef HKY_OS_H_INCLUDED
#define HKY_OS_H_INCLUDED

#include    "../core/hky_config.h"
#include    "../core/hky_core.h"

#define HKY_IO_SENDFILE 1

/*
* 这几个函数指针结合下面的结构体使用,根据不同的操作系统进行初始化操作.
* 不同的操作系统初始化操作在hky_XXX_init.c(XXX表示操作系统名称)文件中,通过hky_os_specific_init函数进行初始化工作
*/
typedef ssize_t (*hky_recv_pt)(hky_connection_t *c,hky_uchar *buf,size_t size);
typedef ssize_t (*hky_recv_chain_pt)(hky_connection_t *c,hky_chain_t *in, off_t limit);
typedef ssize_t (*hky_send_pt)(hky_connection_t *c,hky_uchar *buf,size_t size);
typedef hky_chain_t *(*hky_send_chain_pt)(hky_connection_t *c,hky_chain_t *in,off_t limit);

typedef struct{
    hky_recv_pt recv;
    hky_recv_chain_pt recv_chain;
    hky_recv_pt udp_recv;
    hky_send_pt send;
    hky_send_pt udp_send;
    hky_send_chain_pt udp_send_chain;
    hky_send_chain_pt send_chain;
    hky_uint_t flags;
}hky_os_io_t;

/*
* 对所有操作系统进行统一初始化工作
*/
hky_int_t   hky_os_init(hky_log_t *log);
/*
* 对单独操作系统进行分别初始化
*/
hky_int_t   hky_os_specific_init(hky_log_t  *log);

#endif // HKY_OS_H_INCLUDED
