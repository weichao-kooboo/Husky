#ifndef HKY_TIMES_H_INCLUDED
#define HKY_TIMES_H_INCLUDED

#include "hky_config.h"
#include "hky_core.h"

typedef struct {
    time_t sec;
    hky_uint_t msec;
    hky_int_t gmtoff;
} hky_time_t;

/*
*   用于初始化所有显示的时间,
*   显示的时间字符串分为5种:
*   err_log,http,http_log,http_log_iso,syslog,
*   这5种时间使用数组缓存起来,列表示字符串长度,行表示字符串
*/
void hky_time_init(void);
/*
*   取出当前时间,如果缓存数组中没有就从系统中取出,如果缓存中
*   有就直接取缓存
*   使用gettimeofday取出的时候GMT(中央时区)时间,并不是本地时间,需要根据
*   时区本地化
*   需要调用该函数同步时间
*/
void hky_time_update(void);
void hky_time_sigsafe_update(void);
hky_uchar *hky_http_time(hky_uchar *buf,time_t t);
hky_uchar *hky_http_cookie_time(hky_uchar *buf,time_t t);
/*
*   通过t算出GMT(中央时区)时间
*/
void hky_gmtime(time_t t,hky_tm_t *tp);

time_t hky_next_time(time_t when);
#define hky_next_time_n "mktime()"

extern volatile hky_time_t *hky_cached_time;

#define hky_time() hky_cached_time->sec
#define hky_timeofday() (hky_time_t*)hky_cached_time

/*
*   缓存err_log_time
*/
extern volatile hky_str_t hky_cached_err_log_time;
/*
*   缓存http_time
*/
extern volatile hky_str_t hky_cached_http_time;
/*
*   缓存http_log_time
*/
extern volatile hky_str_t hky_cached_http_log_time;
/*
*   缓存err_log_iso
*/
extern volatile hky_str_t hky_cached_http_log_iso8601;
/*
*   缓存syslog_time
*/
extern volatile hky_str_t hky_cached_syslog_time;


extern volatile hky_msec_t hky_current_msec;

#endif // HKY_TIMES_H_INCLUDED
