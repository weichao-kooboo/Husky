#ifndef HKY_SETPROCTITLE_H_INCLUDED
#define HKY_SETPROCTITLE_H_INCLUDED

#if (HKY_HAVE_SETPROCTITLE)
#define hky_init_setproctitle(log)  HKY_OK
#define hky_setproctitle(title) setproctitle("%s",title);
#else
#if !defined    HKY_SETPROCTITLE_USES_ENV
#if (HKY_SOLARIS)
#define HKY_SETPROCTITLE_USES_ENV   1
#define HKY_SETPROCTITLE_PAD    ' '

hky_int_t   hky_init_setproctitle(hky_log_t *log);
void hky_setproctitle(char *title);
#elif(HKY_LINUX)||(HKY_DARWIN)
#define HKY_SETPROCTITLE_USES_ENV   1
#define HKY_SETPROCTITLE_PAD    '\0'

hky_int_t   hky_init_setproctitle(hky_log_t *log);
void hky_setproctitle(char *title);
#else
#define hky_init_setproctitle(log)  HKY_OK
#define hky_setproctitle(title)
#endif

#endif

#endif

#endif // HKY_SETPROCTITLE_H_INCLUDED
