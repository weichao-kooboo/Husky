
#include "hky_config.h"
#include "hky_core.h"

#define HKY_TIME_SLOTS 64

static hky_uint_t slot;
static hky_atomic_t hky_time_lock;

volatile hky_msec_t      hky_current_msec;
volatile hky_time_t     *hky_cached_time;
volatile hky_str_t       hky_cached_err_log_time;
volatile hky_str_t       hky_cached_http_time;
volatile hky_str_t       hky_cached_http_log_time;
volatile hky_str_t       hky_cached_http_log_iso8601;
volatile hky_str_t       hky_cached_syslog_time;

#if !(HKY_WIN32)
static hky_int_t cached_gmtoff;
#endif

static hky_time_t cached_time[HKY_TIME_SLOTS];
static hky_uchar cached_err_log_time[HKY_TIME_SLOTS]
                                                                                        [sizeof("1970/09/28 12:00:00")];
static hky_uchar cached_http_time[HKY_TIME_SLOTS]
                                                                                [sizeof("Mon, 28 Sep 1970 06:00:00 GMT")];
static hky_uchar cached_http_log_time[HKY_TIME_SLOTS]
                                                                                            [sizeof("28/Sep/1970:12:00:00 +0600")];
static hky_uchar cached_http_log_iso8601[HKY_TIME_SLOTS]
                                                                                                [sizeof("1970-09-28T12:00:00+06:00")];
static hky_uchar cached_syslog_time[HKY_TIME_SLOTS]
                                                                                        [sizeof("Sep 28 12:00:00")];

static char  *week[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
static char  *months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                           "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

void
hky_time_init(void){
    hky_cached_err_log_time.len = sizeof("1970/09/28 12:00:00") - 1;
    hky_cached_http_time.len = sizeof("Mon, 28 Sep 1970 06:00:00 GMT") - 1;
    hky_cached_http_log_time.len = sizeof("28/Sep/1970:12:00:00 +0600") - 1;
    hky_cached_http_log_iso8601.len = sizeof("1970-09-28T12:00:00+06:00") - 1;
    hky_cached_syslog_time.len = sizeof("Sep 28 12:00:00") - 1;

    hky_cached_time = &cached_time[0];

    hky_time_update();
}
void
hky_time_update(void){
    hky_uchar *p0,*p1,*p2,*p3,*p4;
    hky_tm_t tm,gmt;
    time_t sec;
    hky_uint_t msec;
    hky_time_t *tp;
    struct timeval tv;

    if(!hky_trylock(&hky_time_lock)){
        return;
    }
    hky_gettimeofday(&tv);

    sec=tv.tv_sec;
    msec=tv.tv_usec/1000;

    hky_current_msec=(hky_msec_t)sec*1000+msec;

    tp=&cached_time[slot];

    if(tp->sec==sec){
        tp->msec=msec;
        hky_unlock(&hky_time_lock);
        return;
    }

    if(slot==HKY_TIME_SLOTS-1){
        slot=0;
    }else{
        slot++;
    }
    tp=&cached_time[slot];

    tp->sec=sec;
    tp->msec=msec;

    hky_gmtime(sec,&gmt);

    p0=&cached_http_time[slot][0];

    (void)hky_sprintf(p0,"%s, %02d %s %4d %02d:%02d:%02d GMT",
                                            week[gmt.hky_tm_wday],gmt.hky_tm_mday,
                                            months[gmt.hky_tm_mon-1],gmt.hky_tm_year,
                                            gmt.hky_tm_hour,gmt.hky_tm_min,gmt.hky_tm_sec);
#if (HKY_HAVE_GETTIMEZONE)
        tp->gmtoff=hky_gettimezour();
        hky_gmtime(sec+tp->gmtoff*60,&tm);
#elif (HKY_HAVE_GMTOFF)
        hky_localtime(sec,&tm);
        cached_gmtoff=(hky_int_t)(tm.hky_tm_gmtoff/60);
        tp->gmtoff=cached_gmtoff;
#else
    hky_localtime(sec,&tm);
    cached_gmtoff=hky_timezone(tm.hky_tm_isdst);
    tp->gmtoff=cached_gmtoff;
#endif
    p1=&cached_err_log_time[slot][0];

    (void)hky_sprintf(p1,"%4d/%02d/%02d %02d:%02d:%02d",
                                                tm.hky_tm_year,tm.hky_tm_mon,
                                                tm.hky_tm_mday,tm.hky_tm_hour,
                                                tm.hky_tm_min,tm.hky_tm_sec);
     p2=&cached_http_log_time[slot][0];

     (void)hky_sprintf(p2,"%02d/%s/%d:%02d:%02d:%02d %c%02i%02i",
                                    tm.hky_tm_mday,months[tm.hky_tm_mon-1],
                                    tm.hky_tm_year,tm.hky_tm_hour,
                                    tm.hky_tm_min,tm.hky_tm_sec,
                                    tp->gmtoff<0?'-':'+',
                                    hky_abs(tp->gmtoff/60),hky_abs(tp->gmtoff%60));
    p3=&cached_http_log_iso8601[slot][0];
    (void)hky_sprintf(p3,"%4d-%02d-%02dT%02d:%02d:%02d%c%02i:%02i",
                                    tm.hky_tm_year,tm.hky_tm_mon,
                                    tm.hky_tm_mday,tm.hky_tm_hour,
                                    tm.hky_tm_min,tm.hky_tm_sec,
                                    tp->gmtoff<0?'-':'+',
                                    hky_abs(tp->gmtoff/60),hky_abs(tp->gmtoff%60));
    p4=&cached_syslog_time[slot][0];
    (void)hky_sprintf(p4,"%s %2d %02d:%02d:%02d",
                                months[tm.hky_tm_mon-1],tm.hky_tm_mday,
                                tm.hky_tm_hour,tm.hky_tm_min,tm.hky_tm_sec);
    hky_memory_barrier();

    hky_cached_time=tp;
    hky_cached_http_time.data=p0;
    hky_cached_err_log_time.data=p1;
    hky_cached_http_log_time.data=p2;
    hky_cached_http_log_iso8601.data=p3;
    hky_cached_syslog_time.data=p4;

    hky_unlock(&hky_time_lock);
}
void
hky_gmtime(time_t t,hky_tm_t *tp){
    hky_int_t yday;
    hky_uint_t sec,min,hour,mday,mon,year,wday,days,leap;
    if(t<0){
        t=0;
    }
    days=t/86400;
    sec=t%86400;

    if(days>2932896){
        days=2932896;
        sec=86399;
    }
    wday=(4+days)%7;

    hour=sec/3600;
    sec%=3600;
    min=sec/60;
    sec%=60;

    days=days-(31+28)+719527;

    year=(days+2)*400/(365*400+100-4+1);
    yday=days-(365*year+year/4-year/100+year/400);
    if(yday<0){
        leap=(year%4==0)&&(year%100||(year%400==0));
        yday=365+leap+yday;
        year--;
    }

    mon=(yday+31)*10/306;

    mday=yday-(367*mon/12-30)+1;

    if(yday>=306){
        year++;
        mon-=10;
    }else{
        mon+=2;
    }

    tp->hky_tm_sec=(hky_tm_sec_t)sec;
    tp->hky_tm_min=(hky_tm_min_t)min;
    tp->hky_tm_hour=(hky_tm_hour_t)hour;
    tp->hky_tm_mday=(hky_tm_mday_t)mday;
    tp->hky_tm_mon=(hky_tm_mon_t)mon;
    tp->hky_tm_year=(hky_tm_year_t)year;
    tp->hky_tm_wday=(hky_tm_wday_t)wday;
}
