
#include    "../core/hky_config.h"
#include    "../core/hky_core.h"
#include    "../core/husky.h"

hky_int_t hky_ncpu;
hky_int_t hky_max_sockets;
hky_uint_t  hky_inherited_nonblocking;
hky_uint_t  hky_tcp_nodelay_and_tcp_nopush;

struct rlimit rlmt;

hky_os_io_t hky_os_io = {
	hky_unix_recv,
	hky_ready_chain,
	hky_udp_unix_recv,
	hky_unix_send,
	hky_udp_unix_send,
	hky_udp_unix_sendmsg_chain,
	hky_writev_chain,
	0
};

hky_int_t
hky_os_init(hky_log_t *log){
    hky_time_t  *tp;
    hky_uint_t  n;
#if (HKY_HAVE_OS_SPECIFIC_INIT)
    if(hky_os_specific_init(log)!=HKY_OK){
        return HKY_ERROR;
    }
#endif
    if(hky_init_setproctitle(log)!=HKY_OK){
        return HKY_ERROR;
    }
    
    hky_pagesize=getpagesize();
    hky_cacheline_size=HKY_CPU_CACHE_LINE;
    
    for (n=hky_pagesize; n>>=1; hky_pagesize_shift++) {
        
    }
    
#if (HKY_HAVE_SC_NPROCESSORS_ONLN)
    if (hky_ncpu==0) {
        hky_ncpu=sysconf(_SC_NPROCESSORS_ONLN);
    }
#endif
    
    if (hky_ncpu<1) {
        hky_ncpu=1;
    }
    hky_cpuinfo();
    
    if (getrlimit(RLIMIT_NOFILE, &rlmt)==-1) {
        hky_log_error(HKY_LOG_ALERT, log, errno,
                      "getrlimit(RLIMIT_NOFILE) failed");
        return HKY_ERROR;
    }
    hky_max_sockets=(hky_int_t)rlmt.rlim_cur;
#if (HKY_HAVE_INHERITED_NONBLOCK||HKY_HAVE_ACCEPT4)
    hky_inherited_nonblocking=1;
#else
    hky_inherited_nonblocking=0;
#endif
    tp=hky_timeofday();
    srandom(((unsigned)hky_pid<<16)^tp->sec^tp->msec);
    return HKY_OK;
}
