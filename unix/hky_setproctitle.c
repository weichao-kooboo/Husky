
#include    "../core/hky_config.h"
#include    "../core/hky_core.h"

#if (HKY_SETPROCTITLE_USES_ENV)

extern char **environ;
static char *hky_os_argv_last;

hky_int_t
hky_init_setproctitle(hky_log_t *log){
    hky_uchar   *p;
    size_t  size;
    hky_uint_t  i;

    size=0;

    for(i=0;environ[i];i++){
        size+=hky_strlen(environ[i])+1;
    }
    p=hky_alloc(size,log);
    if(NULL==p){
        return HKY_ERROR;
    }

    hky_os_argv_last=hky_os_argv[0];

    for(i=0;hky_os_argv[i];i++){
        if(hky_os_argv_last==hky_os_argv[i]){
            hky_os_argv_last=hky_os_argv[i]+hky_strlen(hky_os_argv[i])+1;
        }
    }

    for(i=0;environ[i];i++){
        if(hky_os_argv_last==environ[i]){
            size=hky_strlen(environ[i])+1;
            hky_os_argv_last=environ[i]+size;

            hky_cpystrn(p,(hky_uchar*)environ[i],size);
            environ[i]=(char*)p;
            p+=size;
        }
    }
    hky_os_argv_last--;
    return HKY_OK;
}
void
hky_setproctitle(char *title){
    hky_uchar *p;
#if(HKY_SOLARIS)
    hky_int_t i;
    size_t  size;
#endif
    hky_os_argv[1]=NULL;
    p=hky_cpystrn((hky_uchar*)hky_os_argv[0],(hky_uchar*)"husky:",
                hky_os_argv_last-hky_os_argv[0]);
    p=hky_cpystrn(p,(hky_uchar*)title,hky_os_argv_last-(char*)p);
#if(HKY_SOLARIS)
    size=0;
    for(i=0;i<hky_argc;i++){
        size+=hky_strlen(hky_argv[i])+1;
    }
    if(size>(size_t)((char*)p-hky_os_argv[0])){
        p=hky_cpystrn(p,(hky_uchar*)" (",hky_os_argv_last-(char*)p);
        for(i=0;i<hky_argc;i++){
            p=hky_cpystrn(p,(hky_uchar*)hky_argv[i],
                        hky_os_argv_last-(char*)p);
            p=hky_cpystrn(p,(hky_uchar*)" ",hky_os_argv_last-(char*)p);
        }
        if(*(p-1)==' '){
            *(p-1)=')';
        }
    }
#endif
    if(hky_os_argv_last-(char*)p){
        hky_memset(p,HKY_SETPROCTITLE_PAD,hky_os_argv_last-(char*)p);
    }
    hky_log_debug1(HKY_LOG_DEBUG_CORE,hky_cycle->log,0,"setproctitle: \"%s\"",hky_os_argv[0]);
}
#endif
