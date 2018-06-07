
#include "hky_config.h"
#include "hky_core.h"

static hky_int_t hky_test_full_name(hky_str_t *name);

hky_int_t
hky_get_full_name(hky_pool_t *pool,hky_str_t *prefix,hky_str_t *name){
    size_t len;
    hky_uchar *p,*n;
    hky_int_t rc;

    rc=hky_test_full_name(name);
    if(rc==HKY_OK){
        return rc;
    }
    len=prefix->len;
    #if (HKY_WIN32)
        if(rc==2){
            len=rc;
        }
    #endif
    n=hky_pnalloc(pool,len+name->len+1);
    if(NULL==n){
        return HKY_ERROR;
    }
    p=hky_cpymem(n,prefix->data,len);
    hky_cpystrn(p,name->data,name->len+1);

    name->len+=len;
    name->data=n;

    return HKY_OK;
}
static hky_int_t
hky_test_full_name(hky_str_t *name){
    #if(HKY_WIN32)
        hky_uchar c0,c1;
        c0=name->data[0];

        if(name->len<2){
            if(c0=='/'){
                return 2;
            }
            return HKY_DECLINED;
        }
        c1=name->data[1];
        if(c1==':'){
            c|=0x20;
            if((c0>='a'&&c0<='z')){
                return HKY_OK;
            }
            return HKY_DECLINED;
        }
        if(c1=='/'){
            return HKY_OK;
        }
        if(c0=='/'){
            return 2;
        }
        return HKY_DECLINED;
    #else
        if(name->data[0]=='/'){
            return HKY_OK;
        }
        return HKY_DECLINED;
    #endif
}
hky_int_t 
hky_create_paths(hky_cycle_t *cycle, hky_uid_t user) {
	hky_err_t err;
	hky_uint_t	i;
	hky_path_t	**path;

	path = cycle->paths.elts;
	for (i = 0; i < cycle->paths.nelts; i++)
	{
		if (hky_create_dir(path[i]->name.data, 0700) == HKY_FILE_ERROR) {
			err = hky_errno;
			if (err != HKY_EEXIST) {
				hky_log_error(HKY_LOG_EMERG, cycle->log, err,
					hky_create_dir_n "\"%s\" failed",
					path[i]->name.data);
				return HKY_ERROR;
			}
		}
		if (user == (hky_uid_t)HKY_CONF_UNSET_UINT) {
			continue;
		}

#if !(HKY_WIN32)
		{
			hky_file_info_t	fi;
			if (hky_file_info(path[i]->name.data, &fi) == HKY_FILE_ERROR) {
				hky_log_error(HKY_LOG_EMERG, cycle->log, hky_errno,
					hky_file_info_n "\"%s\" failed", path[i]->name.data);
				return HKY_ERROR;
			}
			if (fi.st_uid != user) {
				if (chown((const char *)path[i]->name.data, user, -1) == -1) {
					hky_log_error(HKY_LOG_EMERG, cycle->log, hky_errno,
						"chown(\"%s\",%d) failed",
						path[i]->name.data, user);
					return HKY_ERROR;
				}
			}
			if ((fi.st_mode&(S_IRUSR | S_IWUSR | S_IXUSR)) != (S_IRUSR | S_IWUSR | S_IXUSR)) {
				fi.st_mode |= (S_IRUSR | S_IWUSR | S_IXUSR);
				if (chmod((const char *)path[i]->name.data, fi.st_mode) == -1) {
					hky_log_error(HKY_LOG_EMERG, cycle->log, hky_errno,
						"chmod() \"%s\" failed", path[i]->name.data);
					return HKY_ERROR;
				}
			}
		}
#endif // !(HKY_WIN32)
	}
	return HKY_OK;
}