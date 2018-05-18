#include "hky_config.h"
#include "hky_core.h"
//保存输入命令参数
static hky_int_t hky_save_argv(hky_cycle_t *cycle,int argc,char *const *argv);
static hky_int_t hky_process_options(hky_cycle_t    *cycle);

static hky_int_t hky_add_inherited_sockets(hky_cycle_t *cycle);
//标记显示帮助
static hky_uint_t hky_show_help;
//标记显示版本
static hky_uint_t hky_show_version;
//标记显示配置
static hky_uint_t   hky_show_configure;
//记录前缀路径
static hky_uchar    *hky_prefix;
//记录配置路径
static hky_uchar    *hky_conf_file;
//记录global参数
static hky_uchar    *hky_conf_params;
//记录信号
static hky_char     *hky_signal;


static char **hky_os_environ;

int main(int argc,char *const *argv){
    hky_buf_t *b;
    hky_log_t *log;
    hky_uint_t i;
    hky_cycle_t *cycle,init_cycle;
    //日志记录2:输出控制台
    hky_log_stderr(0,"hello log");
    char *prefix="/home/wc";
    //初始化时间,并进行缓存和同步
    hky_time_init();
    //获取pid
    hky_pid= hky_getpid();
    //初始化日志结构
    log=hky_log_init(prefix);
    if(log==NULL){
        return 1;
    }
    hky_memzero(&init_cycle,sizeof(hky_cycle_t));
    init_cycle.log=log;
    hky_cycle=&init_cycle;

    init_cycle.pool=hky_create_pool(1024,log);
    if(init_cycle.pool==NULL){
        return 1;
    }

    if(hky_save_argv(&init_cycle,argc,argv)!=HKY_OK){
        return 1;
    }

    //日志用法1:记录文件
    hky_log_error(HKY_LOG_NOTICE,log,0,
                                "using prefix from \"%s\" ",prefix);


    return 0;
}

static hky_int_t
hky_save_argv(hky_cycle_t *cycle,int argc,char *const *argv){
    #if(HKY_FREEBSD)
        hky_os_argv=(char**)argv;
        hky_argc=argc;
        hky_argv=(char**)argv;
    #else
        size_t  len;
        hky_int_t   i;

        hky_os_argv=(char **)argv;
        hky_argc=argc;

        hky_argv=hky_alloc((argc+1)*sizeof(char*),cycle->log);
        if(hky_argv==NULL){
            return HKY_ERROR;
        }

        for(i=0;i<argc;i++){
            len=hky_strlen(argv[i])+1;

            hky_argv[i]=hky_alloc(len,cycle->log);
            if(hky_argv[i]==NULL){
                return HKY_ERROR;
            }
            (void)hky_cpystrn((hky_uchar*)hky_argv[i],(hky_uchar*)argv[i],len);
        }
        hky_argv[i]=NULL;
    #endif
    hky_os_environ=environ;

    return HKY_OK;
}

static hky_int_t
hky_process_options(hky_cycle_t    *cycle){
    hky_uchar   *p;
    size_t  len;

    if(hky_prefix){
        len=hky_strlen(hky_prefix);
        p=hky_prefix;

        //前缀路径不是以'/'结尾,加上'/'
        if(len&&!hky_path_separator(p[len-1])){
            p=hky_pnalloc(cycle->pool,len+1);
            if(NULL==p){
                return HKY_ERROR;
            }
            hky_memcpy(p,hky_prefix,len);
            p[len++]='/';
        }
        cycle->conf_prefix.len=len;
        cycle->conf_prefix.data=p;
        cycle->prefix.len=len;
        cycle->prefix.data=p;
    }else{
        #ifndef HKY_PREFIX
        p=hky_pnalloc(cycle->pool,HKY_MAX_PATH);
        if(NULL==p){
            return  HKY_ERROR;
        }
        if(hky_getcwd(p,HKY_MAX_PATH)==0){
            hky_log_stderr(hky_errno,"[emerg]: " hky_getcwd_n " failed");
            return HKY_ERROR;
        }
        len=hky_strlen(p);
        p[len++]='/';
        cycle->conf_prefix.len=len;
        cycle->conf_prefix.data=p;
        cycle->prefix.len=len;
        cycle->prefix.data=p;
        #else
        #ifdef  HKY_CONF_PREFIX
        hky_str_set(&cycle->conf_prefix,NGX_CONF_PREFIX);
        #else
        hky_str_set(&cycle->conf_prefix,NGX_PREFIX);
        #endif // HKY_CONF_PREFIX
        hky_str_set(&cycle->prefix,NGX_PREFIX);
        #endif // HKY_PREFIX
    }
    if(hky_conf_file){
        cycle->conf_file.len=hky_strlen(hky_conf_file);
        cycle->conf_file.data=hky_conf_file;
    }else{
        hky_str_set(&cycle->conf_file,HKY_CONF_PATH);
    }
    if(hky_conf_full_name(cycle,&cycle->conf_file,0)!=HKY_OK){
        return HKY_ERROR;
    }
    for(p=cycle->conf_file.data+cycle->conf_file.len-1;
        p>cycle->conf_file.data;
        p--){
        if(hky_path_separator(*p)){
            cycle->conf_prefix.len=p-hky_cycle->conf_file.data+1;
            cycle->conf_prefix.data=hky_cycle->conf_file.data;
            break;
        }
        }
        if(hky_conf_params){
            cycle->conf_param.len=hky_strlen(hky_conf_params);
            cycle->conf_param.data=hky_conf_params;
        }

        if(hky_test_config){
            cycle->log->log_level=HKY_LOG_INFO;
        }
        return HKY_OK;
}


static hky_int_t hky_add_inherited_sockets(hky_cycle_t *cycle) {
	hky_char *p, *v, *inherited;
	hky_int_t s;
	hky_listening_t *ls;

	inherited = (hky_uchar*)genenv(HUSKY_VAR);

	if (inherited == NULL) {
		return HKY_OK;
	}

	hky_log_error(HKY_LOG_NOTICE, cycle->log, 0,
		"using inherited sockets from \"%s\"", inherited);

	if (hky_array_init(&cycle->listening, cycle->pool, 10, sizeof(hky_listening_t)) != HKY_OK) {
		return HKY_ERROR;
	}
	
	for (p = inherited, v = p; *p; p++) {
		if (*p == ':' || *p == ';') {
			s = hky_atoi(v, p - v);
			if (s == HKY_ERROR) {
				hky_log_error(HKY_LOG_EMERG, cycle->log, 0,
					"invalid socket number \"%s\" in  " HUSKY_VAR
					"environment variable,ignoring the rest of the variable", v);
				break;
			}
			v = p + 1;
			ls = hky_array_push(&cycle->listening);
			if (ls == NULL) {
				return HKY_ERROR;
			}
			hky_memzero(ls, sizeof(hky_listening_t));
			ls->fd = (hky_socket_t)s;
		}
	}
	if (v != p) {
		hky_log_error(HKY_LOG_EMERG, cycle->log, 0,
			"invalid socket number \"%s\" in " HUSKY_VAR
			" environment variable,ignoring", v);
	}
	hky_inherited = 1;

	return hky_set_inherited_sockets(cycle);
}
