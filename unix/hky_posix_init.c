
#include    "../core/hky_config.h"
#include    "../core/hky_core.h"
#include    "../core/husky.h"

hky_int_t
hky_os_init(hky_log_t *log){
    hky_time_t  *tp;
    hky_uint_t  n;
#if (HKY_HAVE_OS_SPECIFIC_INIT)
    if(hky_os_specific_init(log)!=HKY_OK){
        return HKY_ERROR;
    }
#endif
    if(hky_init_setproctitle(log)!=HKY_OK){
        return HKY_ERROR;
    }
}
