#ifndef HKY_THREAD_H_INCLUDED
#define HKY_THREAD_H_INCLUDED

#include "../core/hky_config.h"
#include "../core/hky_core.h"

#if(HKY_THREADS)

#if (HKY_LINUX)

typedef pid_t hky_tid_t;
#define HKY_TID_T_FMT "%P"

#elif (HKY_FREEBSD)

typedef uint32_t hky_tid_t;
#define HKY_TID_T_FMT "%uD"

#elif (HKY_DARWIN)

typedef uint64_t hky_tid_t;
#define HKY_TID_T_FMT "%uA"

#else

typedef uint64_t hky_tid_t;
#define HKY_TID_T_FMT "%uA"

#endif

hky_tid_t hky_thread_tid(void);
#define hky_log_tid hky_thread_tid();

#else
#define hky_log_tid 0
#define HKY_TID_T_FMT "%d"
#endif

#endif // HKY_THREAD_H_INCLUDED
