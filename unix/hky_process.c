#include "../core/hky_config.h"
#include "../core/hky_core.h"
int hky_argc;
 char    **hky_argv;
 char    **hky_os_argv;


void 
hky_debug_point(void) {
	hky_core_conf_t *ccf;
	ccf = (hky_core_conf_t *)hky_get_conf(hky_cycle->conf_ctx, hky_core_module);

	switch (ccf->debug_points) {
	case HKY_DEBUG_POINTS_STOP:
		raise(SIGSTOP);
		break;
	case HKY_DEBUG_POINTS_ABORT:
		hky_abort();
	}
 }
