#ifndef HKY_CORE_H_INCLUDED
#define HKY_CORE_H_INCLUDED

#include "hky_config.h"

typedef u_char hky_uchar;
typedef u_int hky_uint;
typedef uint32_t hky_uint32;
typedef uint64_t hky_uint64;
typedef u_long hky_ulong;
typedef char hky_char;
typedef int hky_int;

typedef struct hky_module_s hky_module_t;
typedef struct hky_conf_s hky_conf_t;
typedef  struct hky_pool_s hky_pool_t;
typedef struct hky_chain_s hky_chain_t;
typedef struct hky_log_s hky_log_t;
typedef struct hky_open_file_s hky_open_file_t;
typedef struct hky_command_s hky_command_t;
typedef struct hky_file_s hky_file_t;
typedef struct hky_cycle_s hky_cycle_t;
typedef struct hky_connection_s hky_connection_t;

#define HKY_OK 0
#define HKY_ERROR -1
#define HKY_AGAIN -2
#define HKY_BUSY -3
#define HKY_DONE -4
#define HKY_DECLINED -5
#define HKY_ABORT -6

#include "../unix/hky_errno.h"
#include "../unix/hky_atomic.h"
#include "../unix/hky_thread.h"
#include "hky_rbtree.h"
#include "../unix/hky_time.h"
#include "../unix/hky_socket.h"
#include "../unix/hky_process.h"
#include "../unix/hky_process_cycle.h"
#include "hky_string.h"
#include "../unix/hky_shmem.h"
#include "hky_times.h"
#include "../unix/hky_files.h"
#include "hky_log.h"
#include "../unix/hky_alloc.h"
#include "hky_palloc.h"
#include "hky_buf.h"
#include "hky_queue.h"
#include "hky_array.h"
#include "hky_list.h"
#include "hky_cycle.h"
#include "hky_file.h"

#include "hky_inet.h"

#include "hky_conf_file.h"
#include "hky_module.h"

#include "../unix/hky_os.h"
#include "hky_connection.h"
#include "hky_syslog.h"

#define LF     (u_char) '\n'
#define CR     (u_char) '\r'
#define CRLF   "\r\n"


#define hky_abs(value)       (((value) >= 0) ? (value) : - (value))
#define hky_max(val1, val2)  ((val1 < val2) ? (val2) : (val1))
#define hky_min(val1, val2)  ((val1 > val2) ? (val2) : (val1))

void hky_cpuinfo(void);

#if (HKY_HAVE_OPENAT)
#define HKY_DISABLE_SYMLINKS_OFF        0
#define HKY_DISABLE_SYMLINKS_ON         1
#define HKY_DISABLE_SYMLINKS_NOTOWNER   2
#endif

#endif // HKY_CORE_H_INCLUDED
