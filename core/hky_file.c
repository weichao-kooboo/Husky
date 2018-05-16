
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
