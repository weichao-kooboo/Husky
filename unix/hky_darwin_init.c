
#include "../core/hky_config.h"
#include "../core/hky_core.h"

char hky_darwin_kern_ostype[16];
char hky_darwin_kern_osrelease[128];
int hky_darwin_hw_ncpu;
int hky_darwin_kern_ipc_somaxconn;
hky_ulong hky_darwin_net_inet_tcp_sendspace;

hky_uint_t hky_debug_malloc;

static hky_os_io_t hky_darwin_io={
    hky_unix_recv,
    hky_readv_chain,
    hky_udp_unix_recv,
    hky_unix_send,
    hky_udp_unix_send,
    hky_udp_unix_sendmsg_chain,
#if (HKY_HAVE_SENDFILE)
    hky_darwin_sendfile_chain,
    HKY_IO_SENDFILE
#else
    hky_writev_chain,
    0
#endif
};

typedef struct{
    char    *name;
    void    *value;
    size_t  size;
    hky_uint_t  exists;
}sysctl_t;

sysctl_t sysctls[]=
{
    {
        "hw.ncpu",
        &hky_darwin_hw_ncpu,
        sizeof(hky_darwin_hw_ncpu),
        0
    },
    {
        "net.inet.tcp.sendspace",
        &hky_darwin_net_inet_tcp_sendspace,
        sizeof(hky_darwin_net_inet_tcp_sendspace),
        0
    },
    {
        "kern.ipc.somaxconn",
        &hky_darwin_kern_ipc_somaxconn,
        sizeof(hky_darwin_kern_ipc_somaxconn),
        0
    },
    {NULL,NULL,0,0}
};

hky_int_t
hky_os_specific_init(hky_log_t  *log){
    size_t size;
    hky_err_t err;
    hky_uint_t i;

    size=sizeof(hky_darwin_kern_ostype);
    if (sysctlbyname("kern.ostype",hky_darwin_kern_ostype,&size,NULL,0)
        ==-1) {
        err=hky_errno;
        if (err!=HKY_ENOENT) {
            hky_log_error(HKY_LOG_ALERT, log, err, "sysctlbyname(kern.ostype) failed");
            
            if (err!=HKY_ENOMEM) {
                return  HKY_ERROR;
            }
            hky_darwin_kern_ostype[size-1]='\0';
        }
    }
    size=sizeof(hky_darwin_kern_osrelease);
    if (sysctlbyname("kern.osrelease", hky_darwin_kern_osrelease, &size, NULL, 0)
        ==-1) {
        err=hky_errno;
        if (err!=HKY_ENOENT) {
            hky_log_error(HKY_LOG_ALERT, log, err, "sysctlbyname(kern.ostype) failed");
            if (err!=HKY_ENOMEM) {
                return HKY_ERROR;
            }
            hky_darwin_kern_osrelease[size-1]='\0';
        }
    }
    for (i=0; sysctls[i].name; i++) {
        size=sysctls[i].size;
        if (sysctlbyname(sysctls[i].name, sysctls[i].value, &size, NULL, 0)
            ==0) {
            sysctls[i].exists=1;
            continue;
        }
        err=hky_errno;
        if (err==HKY_ENOENT) {
            continue;
        }
        hky_log_error(HKY_LOG_ALERT, log, err,
                      "sysctlbyname(%s) failed",sysctls[i].name);
        return HKY_ERROR;
    }
    hky_ncpu=hky_darwin_hw_ncpu;
    if (hky_darwin_kern_ipc_somaxconn>32767) {
        hky_log_error(HKY_LOG_ALERT, log, 0,
                      "sysctl kern.ipc.somaxconn must be less than 32767");
        return HKY_ERROR;
    }
    hky_tcp_nodelay_and_tcp_nopush=1;
    hky_os_io=hky_darwin_io;
    return HKY_OK;
}
