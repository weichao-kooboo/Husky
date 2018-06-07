#ifndef HKY_CONF_FILE_H_INCLUDED
#define HKY_CONF_FILE_H_INCLUDED

#include "hky_config.h"
#include "hky_core.h"

#define	HKY_CONF_NOARGS	0x00000001
#define	HKY_CONF_TAKE1	0x00000002
#define	HKY_CONF_TAKE2	0x00000004
#define	HKY_CONF_TAKE3	0x00000008
#define HKY_CONF_TAKE4	0x00000010
#define HKY_CONF_TAKE5	0x00000020
#define HKY_CONF_TAKE6  0x00000040
#define HKY_CONF_TAKE7  0x00000080

#define HKY_CONF_MAX_ARGS	8

#define HKY_CONF_BLOCK  0x00000100
#define HKY_CONF_FLAG	0x00000200
#define	HKY_CONF_ANY	0x00000400
#define HKY_CONF_1MORE	0x00000800
#define	HKY_CONF_2MORE	0x00001000

#define HKY_DIRECT_CONF	0x00010000

#define HKY_MAIN_CONF	0x01000000
#define	HKY_ANY_CONF	0x1F000000

#define HKY_CONF_UNSET	-1
#define	HKY_CONF_UNSET_UINT	(hky_uint_t)-1
#define	HKY_CONF_UNSET_PTR	(void*)-1
#define	HKY_CONF_UNSET_SIZE	(size_t)-1
#define	HKY_CONF_UNSET_MSEC	(hky_msec_t)-1

#define HKY_CONF_OK          NULL
#define HKY_CONF_ERROR       (void *) -1

#define HKY_MAIN_CONF 0x01000000
#define HKY_ANY_CONF 0x1F000000

#define HKY_CONF_BLOCK_START 1
#define HKY_CONF_BLOCK_DONE 2
#define HKY_CONF_FILE_DONE 3

#define HKY_CORE_MODULE 0x45524F43
#define HKY_CONF_MODULE 0x464E4F43

struct hky_command_s{
	hky_str_t	name;
	hky_uint_t	type;
	char	*(*set)(hky_conf_t *cf, hky_command_t *cmd, void *conf);
	hky_uint_t	conf;
	hky_uint_t	offset;
	void	*post;
};

typedef struct {
    hky_file_t file;
    //保存启动时输入的-g参数
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

#define hky_get_conf(conf_ctx,module)	conf_ctx[module.index]

//初始化参数
char *hky_conf_param(hky_conf_t *cf);
//解析配置文件
char *hky_conf_parse(hky_conf_t *cf, hky_str_t *filename);

hky_int_t hky_conf_full_name(hky_cycle_t *cycle,hky_str_t *name,hky_uint_t conf_prefix);
hky_open_file_t *hky_conf_open_file(hky_cycle_t *cycle, hky_str_t *name);
void hky_cdecl hky_conf_log_error(hky_uint_t level,hky_conf_t *cf,hky_err_t err,const char *fmt,...);

#endif // HKY_CONF_FILE_H_INCLUDED
