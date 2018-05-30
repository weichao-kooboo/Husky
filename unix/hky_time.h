#ifndef HKY_TIME_H_INCLUDED
#define HKY_TIME_H_INCLUDED

#include "../core/hky_config.h"
#include "../core/hky_core.h"

typedef hky_rbtree_key_t hky_msec_t;
typedef hky_rbtree_key_int_t hky_msec_int_t;

typedef struct tm hky_tm_t;

#define hky_tm_sec tm_sec
#define hky_tm_min tm_min
#define hky_tm_hour tm_hour
#define hky_tm_mday tm_mday
#define hky_tm_mon tm_mon
#define hky_tm_year tm_year
#define hky_tm_wday tm_wday
#define hky_tm_isdst tm_isdst

#define hky_tm_sec_t    int
#define hky_tm_min_t    int
#define hky_tm_hour_t   int
#define hky_tm_mday_t   int
#define hky_tm_mon_t    int
#define hky_tm_year_t   int
#define hky_tm_wday_t   int

#if (HKY_HAVE_GMTOFF)
#define hky_tm_gmtoff   tm_gmtoff
#define hky_tm_zone tm_zone
#endif

/*timezone结构体在linux内核中,表示时区*/
#if(HKY_SOLARIS)
#define hky_timezone(isdst) (-(isdst?altzone:timezone)/60)
#else
#define hky_timezone(isdst) (-(isdst?timezone+3600:timezone)/60)
#endif

void hky_timezone_update(void);
void hky_localtime(time_t s,hky_tm_t *tm);
void hky_libc_localtime(time_t s,struct tm *tm);
void hky_libc_gmtime(time_t s,struct tm *tm);

#define hky_gettimeofday(tp)    (void)gettimeofday(tp,NULL)
#define hky_msleep(ms)  (void)usleep(ms*1000)
#define hky_sleep(s)    (void)sleep(s)

#endif // HKY_TIME_H_INCLUDED
