#ifndef HKY_FREEBSD_CONFIG_H_INCLUDED
#define HKY_FREEBSD_CONFIG_H_INCLUDED

//根据不同的操作系统配置环境变量
#include "../core/hky_sys_config.h"

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
#include <time.h>
#include <sys/param.h>          /* ALIGN() */
#include <sys/mount.h>          /* statfs() */

#include <sys/filio.h>          /* FIONBIO */
#include <sys/uio.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sched.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>        /* TCP_NODELAY, TCP_NOPUSH */
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/un.h>

#include <libutil.h>            /* setproctitle() before 4.1 */
#include <osreldate.h>
#include <sys/sysctl.h>

#include <dlfcn.h>


#if __FreeBSD_version < 400017

#undef  CMSG_SPACE
#define CMSG_SPACE(l)       (ALIGN(sizeof(struct cmsghdr)) + ALIGN(l))

#undef  CMSG_LEN
#define CMSG_LEN(l)         (ALIGN(sizeof(struct cmsghdr)) + (l))

#undef  CMSG_DATA
#define CMSG_DATA(cmsg)     ((u_char *)(cmsg) + ALIGN(sizeof(struct cmsghdr)))

#endif


#if (HKY_HAVE_POSIX_SEM)
#include <semaphore.h>
#endif


#if (HKY_HAVE_POLL)
#include <poll.h>
#endif


#if (HKY_HAVE_KQUEUE)
#include <sys/event.h>
#endif


#if (HKY_HAVE_FILE_AIO)
#include <aio.h>
typedef struct aiocb  hky_aiocb_t;
#endif


#define HKY_LISTEN_BACKLOG        -1


#ifdef __DragonFly__
#define HKY_KEEPALIVE_FACTOR      1000
#endif


#ifndef IOV_MAX
#define IOV_MAX   1024
#endif


#ifndef HKY_HAVE_INHERITED_NONBLOCK
#define HKY_HAVE_INHERITED_NONBLOCK  1
#endif


#define HKY_HAVE_OS_SPECIFIC_INIT    1
#define HKY_HAVE_DEBUG_MALLOC        1


extern char **environ;
extern char  *malloc_options;

#endif // HKY_FREEBSD_CONFIG_H_INCLUDED
