#ifndef HKY_ERRNO_H_INCLUDED
#define HKY_ERRNO_H_INCLUDED

#include "../core/hky_config.h"
#include "../core/hky_core.h"

typedef int hky_err_t;


#define HKY_EPERM         EPERM
#define HKY_ENOENT        ENOENT
#define HKY_ENOPATH       ENOENT
#define HKY_ESRCH         ESRCH
#define HKY_EINTR         EINTR
#define HKY_ECHILD        ECHILD
#define HKY_ENOMEM        ENOMEM
#define HKY_EACCES        EACCES
#define HKY_EBUSY         EBUSY
#define HKY_EEXIST        EEXIST
#define HKY_EEXIST_FILE   EEXIST
#define HKY_EXDEV         EXDEV
#define HKY_ENOTDIR       ENOTDIR
#define HKY_EISDIR        EISDIR
#define HKY_EINVAL        EINVAL
#define HKY_ENFILE        ENFILE
#define HKY_EMFILE        EMFILE
#define HKY_ENOSPC        ENOSPC
#define HKY_EPIPE         EPIPE
#define HKY_EINPROGRESS   EINPROGRESS
#define HKY_ENOPROTOOPT   ENOPROTOOPT
#define HKY_EOPNOTSUPP    EOPNOTSUPP
#define HKY_EADDRINUSE    EADDRINUSE
#define HKY_ECONNABORTED  ECONNABORTED
#define HKY_ECONNRESET    ECONNRESET
#define HKY_ENOTCONN      ENOTCONN
#define HKY_ETIMEDOUT     ETIMEDOUT
#define HKY_ECONNREFUSED  ECONNREFUSED
#define HKY_ENAMETOOLONG  ENAMETOOLONG
#define HKY_ENETDOWN      ENETDOWN
#define HKY_ENETUNREACH   ENETUNREACH
#define HKY_EHOSTDOWN     EHOSTDOWN
#define HKY_EHOSTUNREACH  EHOSTUNREACH
#define HKY_ENOSYS        ENOSYS
#define HKY_ECANCELED     ECANCELED
#define HKY_EILSEQ        EILSEQ
#define HKY_ENOMOREFILES  0
#define HKY_ELOOP         ELOOP
#define HKY_EBADF         EBADF


#define hky_errno			errno
#define hky_socket_errno	errno

hky_uchar *hky_strerror(hky_err_t err,hky_uchar *errstr,size_t size);

#endif // HKY_ERRNO_H_INCLUDED
