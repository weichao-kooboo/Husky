#ifndef HKY_PROCESS_CYCLE_H_INCLUDED
#define HKY_PROCESS_CYCLE_H_INCLUDED

#include "../core/hky_config.h"
#include "../core/hky_core.h"

#define HKY_CMD_OPEN_CHANNEL	1
#define	HKY_CMD_CLOSE_CHANNEL	2
#define	HKY_CMD_QUIT			3
#define	HKY_CMD_TERMINATE		4
#define	HKY_CMD_REOPEN			5

#define	HKY_PROCESS_SINGLE		0
#define	HKY_PROCESS_MASTER		1
#define	HKY_PROCESS_SIGNALLER	2
#define	HKY_PROCESS_WORKER		3
#define	HKY_PROCESS_HELPER		4

extern hky_uint_t	hky_process;
extern hky_pid_t	hky_new_binary;
extern hky_pid_t hky_pid;
extern hky_uint_t hky_inherited;

#endif // HKY_PROCESS_CYCLE_H_INCLUDED
