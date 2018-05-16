#ifndef HKY_POSIX_CONFIG_H_INCLUDED
#define HKY_POSIX_CONFIG_H_INCLUDED

//根据不同的操作系统配置环境变量
#include "../core/hky_sys_config.h"

#if (HKY_HPUX)
#define _XOPEN_SOURCE
#define _XOPEN_SOURCE_EXTENDED  1
#define _HPUX_ALT_XOPEN_SOCKET_API
#endif


#if (HKY_TRU64)
#define _REENTRANT
#endif


#if (HKY_GNU_HURD)
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#define _FILE_OFFSET_BITS       64
#endif


#ifdef __CYGWIN__
#define timezonevar
#define HKY_BROKEN_SCM_RIGHTS   1
#endif


#include <sys/types.h>
#include <sys/time.h>
#if (HKY_HAVE_UNISTD_H)
#include <unistd.h>
#endif
#if (HKY_HAVE_INTTYPES_H)
#include <inttypes.h>
#endif
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
#if (HKY_HAVE_SYS_PARAM_H)
#include <sys/param.h>
#endif
#if (HKY_HAVE_SYS_MOUNT_H)
#include <sys/mount.h>
#endif
#if (HKY_HAVE_SYS_STATVFS_H)
#include <sys/statvfs.h>
#endif

#if (HKY_HAVE_SYS_FILIO_H)
#include <sys/filio.h>
#endif
#include <sys/ioctl.h>

#include <sys/uio.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sched.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/un.h>

#if (HKY_HAVE_LIMITS_H)
#include <limits.h>
#endif

#ifdef __CYGWIN__
#include <malloc.h>
#endif

#if (HKY_HAVE_CRYPT_H)
#include <crypt.h>
#endif


#ifndef IOV_MAX
#define IOV_MAX   16
#endif


#if (HKY_HAVE_DLOPEN)
#include <dlfcn.h>
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


#if (HKY_HAVE_DEVPOLL) && !(HKY_TEST_BUILD_DEVPOLL)
#include <sys/ioctl.h>
#include <sys/devpoll.h>
#endif


#if (HKY_HAVE_FILE_AIO)
#include <aio.h>
typedef struct aiocb  ngx_aiocb_t;
#endif


#define HKY_LISTEN_BACKLOG  511

#define hky_debug_init()


#if (__FreeBSD__) && (__FreeBSD_version < 400017)

#include <sys/param.h>          /* ALIGN() */


#undef  CMSG_SPACE
#define CMSG_SPACE(l)       (ALIGN(sizeof(struct cmsghdr)) + ALIGN(l))

#undef  CMSG_LEN
#define CMSG_LEN(l)         (ALIGN(sizeof(struct cmsghdr)) + (l))

#undef  CMSG_DATA
#define CMSG_DATA(cmsg)     ((u_char *)(cmsg) + ALIGN(sizeof(struct cmsghdr)))

#endif


extern char **environ;

#endif // HKY_POSIX_CONFIG_H_INCLUDED
