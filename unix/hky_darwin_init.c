
#include "../core/hky_config.h"
#include "../core/hky_core.h"

char hky_darwin_kern_ostype[16];
char hky_darwin_kern_osrelease[128];
int hky_darwin_hw_ncpu;
int hky_darwin_kern_ipc_somaxconn;
hky_ulong hky_darwin_net_inet_tcp_sendspace;

hky_uint_t hky_debug_malloc;


hky_int_t
hky_os_specific_init(hky_log_t  *log){
    size_t size;
    hky_err_t err;
    hky_uint_t i;

    size=sizeof(hky_darwin)
}
