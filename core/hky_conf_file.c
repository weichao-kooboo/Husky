
#include "hky_config.h"
#include "hky_core.h"

#define HKY_CONF_BUFFER 4096

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
