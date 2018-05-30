#ifndef HKY_PROCESS_H_INCLUDED
#define HKY_PROCESS_H_INCLUDED

#include "hky_setproctitle.h"

typedef pid_t hky_pid_t;

#define hky_getpid getpid

#ifndef hky_log_pid
#define hky_log_pid hky_pid
#endif // hky_log_pid

extern int hky_argc;
extern  char    **hky_argv;
extern  char    **hky_os_argv;

extern hky_pid_t hky_pid;

#endif // HKY_PROCESS_H_INCLUDED
