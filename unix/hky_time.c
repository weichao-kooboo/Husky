#include "../core/hky_config.h"
#include "../core/hky_core.h"

void
hky_timezone_update(void){
#if (HKY_FREEBSD)
    if(getenv("TZ")){
        return;
    }
    putenv("TZ=UTC");
    tzset();
    unsetenv("TZ");
    tzset();
#elif(HKY_LINUX)
    time_t s;
    struct tm *t;
    char buf[4];
    s=time(0);
    t=localtime(&s);
    strftime(buf,4,"%H",t);
#endif
}
void
hky_localtime(time_t s,hky_tm_t *tm){
#if (HKY_HAVE_LOCALTIME_R)
    (void)localtime_r(&s,tm);
#else
    hky_tm_t *t;
    t=localtime(&s);
    *tm=*t;
#endif
    tm->hky_tm_mon++;
    tm->hky_tm_year+=1900;
}
void
hky_libc_localtime(time_t s,struct tm *tm){
#if (HKY_HAVE_LOCALTIME_R)
    (void)localtime_r(&s,tm);
#else
    struct tm *t;
    t=localtime(&s);
    *tm=*t;
#endif
}
void
hky_libc_gmtime(time_t s,struct tm *tm){
#if(HKY_HAVE_LOCALTIME_R)
    (void)gmtime_r(&s,tm);
#else
    struct tm *t;
    t=gmtime(&s);
    *tm=*t;
#endif
}
