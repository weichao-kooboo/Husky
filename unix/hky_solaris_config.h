#ifndef HKY_SOLARIS_CONFIG_H_INCLUDED
#define HKY_SOLARIS_CONFIG_H_INCLUDED



#ifndef _REENTRANT
#define _REENTRANT
#endif

#define _FILE_OFFSET_BITS  64   /* must be before <sys/types.h> */

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
#include <sys/statvfs.h>        /* statvfs() */

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
#include <netinet/tcp.h>        /* TCP_NODELAY */
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/un.h>

#include <sys/systeminfo.h>
#include <limits.h>             /* IOV_MAX */
#include <inttypes.h>
#include <crypt.h>

#include <dlfcn.h>

#define HKY_ALIGNMENT  _MAX_ALIGNMENT


#if (HKY_HAVE_POSIX_SEM)
#include <semaphore.h>
#endif


#if (HKY_HAVE_POLL)
#include <poll.h>
#endif


#if (HKY_HAVE_DEVPOLL)
#include <sys/ioctl.h>
#include <sys/devpoll.h>
#endif


#if (HKY_HAVE_EVENTPORT)
#include <port.h>
#endif


#if (HKY_HAVE_SENDFILE)
#include <sys/sendfile.h>
#endif


#define HKY_LISTEN_BACKLOG           511


#ifndef HKY_HAVE_INHERITED_NONBLOCK
#define HKY_HAVE_INHERITED_NONBLOCK  1
#endif


#ifndef HKY_HAVE_SO_SNDLOWAT
/* setsockopt(SO_SNDLOWAT) returns ENOPROTOOPT */
#define HKY_HAVE_SO_SNDLOWAT         0
#endif


#define HKY_HAVE_OS_SPECIFIC_INIT    1
#define hky_debug_init()


extern char **environ;

#endif // HKY_SOLARIS_CONFIG_H_INCLUDED
