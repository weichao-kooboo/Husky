
#include "hky_config.h"
#include "hky_core.h"

#define HKY_CONF_BUFFER 4096

static hky_int_t hky_conf_add_dump(hky_conf_t *cf,hky_str_t *filename);

hky_open_file_t *
hky_conf_open_file(hky_cycle_t *cycle, hky_str_t *name){
    hky_str_t full;
    hky_uint_t i;
    hky_list_part_t *part;
    hky_open_file_t *file;

    #if (HKY_SUPPRESS_WARN)
        hky_str_null(&full);
    #endif

    if(name->len){
        full=*name;

        if(hky_conf_full_name(cycle,&full,0)!=NULL){
            return NULL;
        }

        part=&cycle->open_files.part;
        file=part->elts;

        for(i=0;;i++){
            if(i>=part->nelts){
                if(part->next==NULL){
                    break;
                }
                part=part->next;
                file=part->elts;
                i=0;
            }
            if(full.len!=file[i].name.len){
                continue;
            }
            if(hky_strcmp(full.data,file[i].name.data)==0){
                return &file[i];
            }
        }
    }
    file=hky_list_push(&cycle->open_files);
    if(NULL==file){
        return NULL;
    }
    if(name->len){
        file->fd=HKY_INVALID_FILE;
        file->name=full;
    }else{
        file->fd=hky_stderr;
        file->name=*name;
    }

    file->flush=NULL;
    file->data=NULL;

    return file;
}
void hky_cdecl
hky_conf_log_error(hky_uint_t level,hky_conf_t *cf,hky_err_t err,const char *fmt,...){
    hky_uchar errstr[HKY_MAX_CONF_ERRSTR],*p,*last;
    va_list args;

    last=errstr + HKY_MAX_CONF_ERRSTR;

    va_start(args,fmt);
    p=hky_vslprintf(errstr,last,fmt,args);
    va_end(args);

    if(err){
        p=hky_log_errno(p,last,err);
    }
    if(cf->conf_file==NULL){
        hky_log_error(level,cf->log,0,"%*s",p-errstr,errstr);
        return;
    }
    if(cf->conf_file->file.fd==HKY_INVALID_FILE){
        hky_log_error(level,cf->log,0,"%*s in command line",p-errstr,errstr);
        return;
    }
    hky_log_error(level,cf->log,0,"%*s in %s:%ui",
                                p-errstr,errstr,
                                cf->conf_file->file.name.data,cf->conf_file->line);
}
hky_int_t
hky_conf_full_name(hky_cycle_t *cycle,hky_str_t *name,hky_uint_t conf_prefix){
    hky_str_t *prefix;
    prefix=conf_prefix? &cycle->conf_prefix: &cycle->prefix;
    return hky_get_full_name(cycle->pool,prefix,name);
}
char *
hky_conf_param(hky_conf_t *cf) {
	char *rv;
	hky_str_t *param;
	hky_buf_t b;
	hky_conf_file_t conf_file;

	param = &cf->cycle->conf_param;

	if (param->len == 0) {
		return HKY_CONF_OK;
	}

	hky_memzero(&conf_file, sizeof(hky_conf_file_t));

	hky_memzero(&b, sizeof(hky_buf_t));

	b.start = param->data;
	b.pos = param->data;
	b.last = param->data + param->len;
	b.end = b.last;
	b.temporary = 1;

	conf_file.file.fd = HKY_INVALID_FILE;
	conf_file.file.name.data = NULL;
	conf_file.line = 0;
    
    cf->conf_file=&conf_file;
    cf->conf_file->buffer=&b;
    
    rv=hky_conf_parse(cf, NULL);
    cf->conf_file=NULL;
    return rv;
}
char *
hky_conf_parse(hky_conf_t *cf, hky_str_t *filename) {
    char *rv;
    hky_fd_t fd;
    hky_int_t rc;
    hky_buf_t buf;
    hky_conf_file_t *prev,conf_file;
    enum{
        parse_file=0,
        parse_block,
        parse_param
    }type;
    
#if (HKY_SUPPRESS_WARN)
    fd=HKY_INVALID_FILE;
    prev=NULL;
#endif
    if (filename) {
        fd=hky_open_file(filename->data, HKY_FILE_RDONLY, HKY_FILE_OPEN, 0);
        if (fd==HKY_INVALID_FILE) {
            hky_conf_log_error(HKY_LOG_EMERG, cf, hky_errno,
                               hky_open_file_n " \"%s\" failed",filename->data);
            return HKY_CONF_ERROR;
        }
        prev=cf->conf_file;
        cf->conf_file=&conf_file;
        
        if (hky_fd_info(fd, &cf->conf_file->file.info)==HKY_FILE_ERROR) {
            hky_log_error(HKY_LOG_EMERG, cf->log, hky_errno,
                          hky_fd_info_n " \"%s\" failed",filename->data);
        }
        
        cf->conf_file->buffer=&buf;
        buf.start=hky_alloc(HKY_CONF_BUFFER, cf->log);
        if (buf.start==NULL) {
            goto failed;
        }
        buf.pos=buf.start;
        buf.last=buf.start;
        buf.end=buf.last+HKY_CONF_BUFFER;
        buf.temporary=1;
        
        cf->conf_file->file.fd  =fd;
        cf->conf_file->file.name.len=filename->len;
        cf->conf_file->file.name.data=filename->data;
        cf->conf_file->file.offset=0;
        cf->conf_file->file.log=cf->log;
        cf->conf_file->line=1;
        
        type=parse_file;
        
        if (hky_dump_config
#if (HKY_DEBUG)
            || 1
#endif
            ) {
            if (hky_conf_add_dump(cf, filename)!=HKY_OK) {
                goto failed;
            }
        }else{
            cf->conf_file->dump=NULL;
        }
    }else if(cf->conf_file->file.fd!=HKY_INVALID_FILE){
        type=parse_block;
    }else{
        type=parse_param;
    }
    
}

static hky_int_t
hky_conf_add_dump(hky_conf_t *cf,hky_str_t *filename){
    off_t size;
    hky_uchar   *p;
    uint32_t hash;
    hky_buf_t   *buf;
    hky_str_node_t  *sn;
    hky_conf_dump_t *cd;
    
    hash=hky_crc32_long(filename->data, filename->len);
    
    sn=hky_str_rbtree_lookup(&cf->cycle->config_dump_rbtree, filename, hash);
    
    if (sn) {
        cf->conf_file->dump=NULL;
        return HKY_OK;
    }
    
    p=hky_pstrdup(cf->cycle->pool, filename);
    if (NULL==p) {
        return HKY_ERROR;
    }
    
    cf=hky_array_push(&cf->cycle->config_dump);
    if (NULL==cd) {
        return HKY_ERROR;
    }
    
    size=hky_file_size(&cf->conf_file->file.info);
    
    buf=hky_create_temp_buf(cf->cycle->pool, (size_t)size);
    if (NULL==buf) {
        return HKY_ERROR;
    }
    
    cd->name.data=p;
    cd->name.len=filename->len;
    cd->buffer=buf;
    
    cf->conf_file->dump=buf;
    
    sn=hky_palloc(cf->temp_pool, sizeof(hky_str_node_t));
    
    if (NULL==sn) {
        return HKY_ERROR;
    }
    sn->node.key=hash;
    sn->str=cd->name;
    
    hky_rbtree_insert(&cf->cycle->config_dump_rbtree, &sn->node);
    
    return HKY_OK;
}