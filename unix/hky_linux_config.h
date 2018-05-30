#ifndef HKY_LINUX_CONFIG_H_INCLUDED
#define HKY_LINUX_CONFIG_H_INCLUDED


//根据不同的操作系统配置环境变量
#include "../core/hky_sys_config.h"

#ifndef _GNU_SOURCE
#define _GNU_SOURCE             /* pread(), pwrite(), gethostname() */
#endif

#define _FILE_OFFSET_BITS  64

#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdarg.h>
#include <stddef.h>             /* offsetof() */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <pwd.h>
#include <grp.h>
#include <dirent.h>
#include <glob.h>
#include <sys/vfs.h>            /* statfs() */

#include <sys/uio.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sched.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>        /* TCP_NODELAY, TCP_CORK */
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/un.h>

#include <time.h>               /* tzset() */
#include <malloc.h>             /* memalign() */
#include <limits.h>             /* IOV_MAX */
#include <sys/ioctl.h>
#include <crypt.h>
#include <sys/utsname.h>        /* uname() */

#include <dlfcn.h>


#if (HKY_HAVE_POSIX_SEM)
#include <semaphore.h>
#endif


#if (HKY_HAVE_SYS_PRCTL_H)
#include <sys/prctl.h>
#endif


#if (HKY_HAVE_SENDFILE64)
#include <sys/sendfile.h>
#else
extern ssize_t sendfile(int s, int fd, int32_t *offset, size_t size);
#define HKY_SENDFILE_LIMIT  0x80000000
#endif


#if (HKY_HAVE_POLL)
#include <poll.h>
#endif


#if (HKY_HAVE_EPOLL)
#include <sys/epoll.h>
#endif


#if (HKY_HAVE_SYS_EVENTFD_H)
#include <sys/eventfd.h>
#endif
#include <sys/syscall.h>
#if (HKY_HAVE_FILE_AIO)
#include <linux/aio_abi.h>
typedef struct iocb  ngx_aiocb_t;
#endif


#define HKY_LISTEN_BACKLOG        511


#ifndef HKY_HAVE_SO_SNDLOWAT
/* setsockopt(SO_SNDLOWAT) returns ENOPROTOOPT */
#define HKY_HAVE_SO_SNDLOWAT         0
#endif


#ifndef HKY_HAVE_INHERITED_NONBLOCK
#define HKY_HAVE_INHERITED_NONBLOCK  0
#endif


#define HKY_HAVE_OS_SPECIFIC_INIT    1
#define hky_debug_init()


extern char **environ;


#endif // HKY_LINUX_CONFIG_H_INCLUDED
