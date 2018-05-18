#ifndef HKY_CONF_FILE_H_INCLUDED
#define HKY_CONF_FILE_H_INCLUDED

#include "hky_config.h"
#include "hky_core.h"

#define HKY_CONF_OK          NULL
#define HKY_CONF_ERROR       (void *) -1

struct hky_command_s{

};

typedef struct {
    hky_file_t file;
    hky_buf_t *buffer;
    hky_buf_t *dump;
    hky_uint_t line;
} hky_conf_file_t;

typedef struct {
	hky_str_t name;
	hky_buf_t *buffer;
} hky_conf_dump_t;

typedef char *(*hky_conf_handler_pt)(hky_conf_t *cf,hky_command_t *dummy,void *conf);

struct hky_conf_s{
    char *name;
    hky_array_t *args;

    hky_cycle_t *cycle;
    hky_pool_t *pool;
    hky_pool_t *temp_pool;
    hky_conf_file_t *conf_file;
    hky_log_t *log;

    void *ctx;
    hky_uint_t module_type;
    hky_uint_t cmd_type;

    hky_conf_handler_pt handler;
    char    *handler_conf;
};

struct hky_open_file_s{
    hky_fd_t fd;
    hky_str_t name;

    void (*flush)(hky_open_file_t *file,hky_log_t *log);
    void *data;
};

#define HKY_MAX_CONF_ERRSTR 1024

hky_int_t hky_conf_full_name(hky_cycle_t *cycle,hky_str_t *name,hky_uint_t conf_prefix);
hky_open_file_t *hky_conf_open_file(hky_cycle_t *cycle, hky_str_t *name);
void hky_cdecl hky_conf_log_error(hky_uint_t level,hky_conf_t *cf,hky_err_t err,const char *fmt,...);

#endif // HKY_CONF_FILE_H_INCLUDED
