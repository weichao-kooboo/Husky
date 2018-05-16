#ifndef HKY_LOG_H_INCLUDED
#define HKY_LOG_H_INCLUDED

#include "hky_config.h"
#include "hky_core.h"

#define HKY_LOG_STDERR 0
#define HKY_LOG_EMERG 1
#define HKY_LOG_ALERT 2
#define HKY_LOG_CRIT 3
#define HKY_LOG_ERR 4
#define HKY_LOG_WARN 5
#define HKY_LOG_NOTICE 6
#define HKY_LOG_INFO 7
#define HKY_LOG_DEBUG 8

#define HKY_LOG_DEBUG_CORE 0X010
#define HKY_LOG_DEBUG_ALLOC 0X020
#define HKY_LOG_DEBUG_MUTEX 0X040
#define HKY_LOG_DEBUG_EVENT 0X080
#define HKY_LOG_DEBUG_HTTP 0X100
#define HKY_LOG_DEBUG_MAIL 0X200
#define HKY_LOG_DEBUG_STREAM 0X400

#define HKY_LOG_DEBUG_FIRST HKY_LOG_DEBUG_CORE
#define HKY_LOG_DEBUG_LAST HKY_LOG_DEBUG_STREAM
#define HKY_LOG_DEBUG_CONNECTION 0X80000000
#define HKY_LOG_DEBUG_ALL 0X7ffffff0

typedef hky_uchar *(*hky_log_handler_pt)(hky_log_t *log,hky_uchar *buf,size_t len);
typedef void (*hky_log_writer_pt) (hky_log_t *log,hky_uint_t level,hky_uchar *buf,size_t len);

struct hky_log_s{
    hky_uint_t log_level;

    hky_open_file_t *file;

    hky_atomic_uint_t connection;

    time_t disk_full_time;

    hky_log_handler_pt handler;
    void *data;

    hky_log_writer_pt writer;
    void *wdata;

    char *action;
    hky_log_t *next;
};

#define HKY_MAX_ERROR_STR 2048

/*
* gcc和c99定义的可变参数放在宏中的区别
*/
#if( HKY_HAVE_C99_VARIADIC_MACROS)
#define HKY_HAVE_VARIADIC_MACROS 1
#define hky_log_error(level,log,...)            \
    if((log)->log_level>=level) hky_log_error_core(level,log,__VA_ARGS__)
void hky_log_error_core(hky_uint_t level,hky_log_t *log,hky_err_t err,const char*fmt,...);
#define hky_log_debug(level,log,...)             \
    if((log)->log_level&level)                                      \
        hky_log_error_core(HKY_LOG_DEBUG,log,__VA_ARGS__)
/*****************************************************/
#elif (HKY_HAVE_GCC_VARIADIC_MACROS)
#define HKY_HAVE_VARIADIC_MACROS 1
#define hky_log_error(level,log,args...)            \
    if((log)->log_level>=level) hky_log_error_core(level,log,args)
void hky_log_error_core(hky_uint_t level,hky_log_t *log,hky_err_t err,const char*fmt,...);
#define hky_log_debug(level,log,args...)             \
    if((log)->log_level&level)                                      \
        hky_log_error_core(HKY_LOG_DEBUG,log,args)
/*****************************************************/
#else
#define HKY_HAVE_VARIADIC_MACROS 0
void hky_cdecl hky_log_error(hky_uint_t level,hky_log_t *log,hky_err_t err,
const char *fmt,...);
void hky_log_error_core(hky_uint_t level,hky_log_t *log,hky_err_t err,const char *fmt,va_list args);
void hky_cdecl hky_log_debug_core(hky_log_t *log,hky_err_t err, const char *fmt,...);
#endif
/*****************************************************/

#if (HKY_DEBUG)
#if(HKY_HAVE_VARIADIC_MACROS)
#define hky_log_debug0(level,log,err,fmt)               \
                    hky_log_debug(level,log,err,fmt)
#define hky_log_debug1(level,log,err,fmt,arg1)               \
                    hky_log_debug(level,log,err,fmt,arg1)
#define hky_log_debug2(level,log,err,fmt,arg1,arg2)               \
                    hky_log_debug(level,log,err,fmt,arg1,arg2)
#define hky_log_debug3(level,log,err,fmt,arg1,arg2,arg3)               \
                    hky_log_debug(level,log,err,fmt,arg1,arg2,arg3)
#define hky_log_debug4(level,log,err,fmt,arg1,arg2,arg3,arg4)               \
                    hky_log_debug(level,log,err,fmt,arg1,arg2,arg3,arg4)
#define hky_log_debug5(level,log,err,fmt,arg1,arg2,arg3,arg4,arg5)               \
                    hky_log_debug(level,log,err,fmt,arg1,arg2,arg3,arg4,arg5)
#define hky_log_debug6(level,log,err,fmt,arg1,arg2,arg3,arg4,arg5,arg6)               \
                    hky_log_debug(level,log,err,fmt,arg1,arg2,arg3,arg4,arg5,arg6)
#define hky_log_debug7(level,log,err,fmt,arg1,arg2,arg3,arg4,arg5,arg6,arg7)               \
                    hky_log_debug(level,log,err,fmt,arg1,arg2,arg3,arg4,arg5,arg6,arg7)
#define hky_log_debug8(level,log,err,fmt,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8)               \
                    hky_log_debug(level,log,err,fmt,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8)
#else
#define hky_log_debug0(level,log,err,fmt)                       \
                if((log)->log_level&level)                                                 \
                    hky_log_debug_core(log,err,fmt)
#define hky_log_debug1(level,log,err,fmt,arg1)                       \
                if((log)->log_level&level)                                                 \
                    hky_log_debug_core(log,err,fmt,arg1)
#define hky_log_debug2(level,log,err,fmt,arg1,arg2)                       \
                if((log)->log_level&level)                                                 \
                    hky_log_debug_core(log,err,fmt,arg1,arg2)
#define hky_log_debug3(level,log,err,fmt,arg1,arg2,arg3)                       \
                if((log)->log_level&level)                                                 \
                    hky_log_debug_core(log,err,fmt,arg1,arg2,arg3)
#define hky_log_debug4(level,log,err,fmt,arg1,arg2,arg3,arg4)                       \
                if((log)->log_level&level)                                                 \
                    hky_log_debug_core(log,err,fmt,arg1,arg2,arg3,arg4)
#define hky_log_debug5(level,log,err,fmt,arg1,arg2,arg3,arg4,arg5)                       \
                if((log)->log_level&level)                                                 \
                    hky_log_debug_core(log,err,fmt,arg1,arg2,arg3,arg4,arg5)
#define hky_log_debug6(level,log,err,fmt,arg1,arg2,arg3,arg4,arg5,arg6)                       \
                if((log)->log_level&level)                                                 \
                    hky_log_debug_core(log,err,fmt,arg1,arg2,arg3,arg4,arg5,arg6)
#define hky_log_debug7(level,log,err,fmt,arg1,arg2,arg3,arg4,arg5,arg6,arg7)                       \
                if((log)->log_level&level)                                                 \
                    hky_log_debug_core(log,err,fmt,arg1,arg2,arg3,arg4,arg5,arg6,arg7)
#define hky_log_debug8(level,log,err,fmt,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8)                       \
                if((log)->log_level&level)                                                 \
                    hky_log_debug_core(log,err,fmt,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8)
#endif
#else
#define hky_log_debug0(level,log,err,fmt)
#define hky_log_debug1(level,log,err,fmt,arg1)
#define hky_log_debug2(level,log,err,fmt,arg1,arg2)
#define hky_log_debug3(level,log,err,fmt,arg1,arg2,arg3)
#define hky_log_debug4(level,log,err,fmt,arg1,arg2,arg3,arg4)
#define hky_log_debug5(level,log,err,fmt,arg1,arg2,arg3,arg4,arg5)
#define hky_log_debug6(level,log,err,fmt,arg1,arg2,arg3,arg4,arg5,arg6)
#define hky_log_debug7(level,log,err,fmt,arg1,arg2,arg3,arg4,arg5,arg6,arg7)
#define hky_log_debug8(level,log,err,fmt,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8)
#endif
/*******************************************************************/
hky_log_t *hky_log_init(hky_uchar *prefix);
void hky_cdecl hky_log_abort(hky_err_t err,const char* fmt,...);
void hky_cdecl hky_log_stderr(hky_err_t err, const char *fmt,...);
hky_uchar *hky_log_errno(hky_uchar *buf,hky_uchar *last,hky_err_t err);
hky_int_t hky_log_open_default(hky_cycle_t *cycle);
hky_int_t hky_log_redirect_stderr(hky_cycle_t *cycle);
hky_log_t *hky_log_get_file_log(hky_log_t *head);
char *hky_log_set_log(hky_conf_t *cf,hky_log_t **head);

static hky_inline   void
hky_write_stderr(char *text){
    (void) hky_write_fd(hky_stderr,text,hky_strlen(text));
}

static hky_inline void
hky_write_stdout(char *text){
    (void) hky_write_fd(hky_stdout,text,hky_strlen(text));
}

extern hky_module_t hky_errlog_module;
extern hky_uint_t hky_use_stderr;

#endif // HKY_LOG_H_INCLUDED
