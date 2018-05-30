#include  "hky_config.h"
#include "hky_core.h"

static hky_uchar *hky_sprintf_num(hky_uchar *buf,hky_uchar *last,hky_uint64 ui64,hky_uchar zero,hky_uint_t hexadecimal,hky_uint_t width);
static void hky_encode_base64_internal(hky_str_t *dst,hky_str_t *src,const hky_uchar *basis,hky_uint_t padding);
static hky_int_t hky_decode_base64_internal(hky_str_t *dst,hky_str_t *src,const hky_uchar *basis);


void
hky_strlow(hky_uchar *dst,hky_uchar *src,size_t n){
    while(n){
        *dst=hky_tolower(*src);
        dst++;
        src++;
        n--;
    }
}

hky_uchar *
hky_cpystrn(hky_uchar *dst,hky_uchar *src,size_t n){
    if(n==0){
        return dst;
    }

    while(--n){
            *dst=*src;
            if(*dst=='\0'){
                return dst;
            }
            dst++;
            src++;
    }
    *dst='\0';
    return dst;
}

hky_uchar *
hky_pstrdup(hky_pool_t *pool,hky_str_t *src){
    hky_uchar *dst;
    dst=hky_pnalloc(pool,src->len);
    if(dst==NULL){
        return NULL;
    }
    hky_memcpy(dst,src->data,src->len);

    return dst;
}
hky_uchar *
hky_cdecl hky_sprintf(hky_uchar *buf,const char *fmt, ...){
    hky_uchar *p;
    va_list args;

    va_start(args,fmt);
    p=hky_vslprintf(buf,(void*)-1,fmt,args);
    va_end(args);

    return p;
}
hky_uchar *
hky_cdecl hky_snprintf(hky_uchar *buf,size_t max, const char *fmt,...){
    hky_uchar *p;
    va_list args;

    va_start(args,fmt);
    p=hky_vslprintf(buf,buf+max,fmt,args);
    va_end(args);

    return p;
}
hky_uchar *
hky_cdecl hky_slprintf(hky_uchar *buf,hky_uchar *last,const char *fmt, ...){
    hky_uchar *p;
    va_list args;

    va_start(args,fmt);
    p=hky_vslprintf(buf,last,fmt,args);
    va_end(args);

    return p;
}

hky_uchar *
hky_vslprintf(hky_uchar *buf,hky_uchar *last,const char *fmt,va_list args){
 hky_uchar                *p, zero;
    int                    d;
    double                 f;
    size_t                 len, slen;
    int64_t                i64;
    uint64_t               ui64, frac;
    hky_msec_t             ms;
    hky_uint_t             width, sign, hex, max_width, frac_width, scale, n;
    hky_str_t             *v;
    hky_variable_value_t  *vv;

    while (*fmt && buf < last) {

        /*
         * "buf < last" means that we could copy at least one character:
         * the plain character, "%%", "%c", and minus without the checking
         */

        if (*fmt == '%') {

            i64 = 0;
            ui64 = 0;

            zero = (hky_uchar) ((*++fmt == '0') ? '0' : ' ');
            width = 0;
            sign = 1;
            hex = 0;
            max_width = 0;
            frac_width = 0;
            slen = (size_t) -1;

            while (*fmt >= '0' && *fmt <= '9') {
                width = width * 10 + (*fmt++ - '0');
            }


            for ( ;; ) {
                switch (*fmt) {

                case 'u':
                    sign = 0;
                    fmt++;
                    continue;

                case 'm':
                    max_width = 1;
                    fmt++;
                    continue;

                case 'X':
                    hex = 2;
                    sign = 0;
                    fmt++;
                    continue;

                case 'x':
                    hex = 1;
                    sign = 0;
                    fmt++;
                    continue;

                case '.':
                    fmt++;

                    while (*fmt >= '0' && *fmt <= '9') {
                        frac_width = frac_width * 10 + (*fmt++ - '0');
                    }

                    break;

                case '*':
                    slen = va_arg(args, size_t);
                    fmt++;
                    continue;

                default:
                    break;
                }

                break;
            }


            switch (*fmt) {

            case 'V':
                v = va_arg(args, hky_str_t *);

                len = hky_min(((size_t) (last - buf)), v->len);
                buf = hky_cpymem(buf, v->data, len);
                fmt++;

                continue;

            case 'v':
                vv = va_arg(args, hky_variable_value_t *);

                len = hky_min(((size_t) (last - buf)), vv->len);
                buf = hky_cpymem(buf, vv->data, len);
                fmt++;

                continue;

            case 's':
                p = va_arg(args, hky_uchar *);

                if (slen == (size_t) -1) {
                    while (*p && buf < last) {
                        *buf++ = *p++;
                    }

                } else {
                    len = hky_min(((size_t) (last - buf)), slen);
                    buf = hky_cpymem(buf, p, len);
                }

                fmt++;

                continue;

            case 'O':
                i64 = (int64_t) va_arg(args, off_t);
                sign = 1;
                break;

            case 'P':
                i64 = (int64_t) va_arg(args, hky_pid_t);
                sign = 1;
                break;

            case 'T':
                i64 = (int64_t) va_arg(args, time_t);
                sign = 1;
                break;

            case 'M':
                ms = (hky_msec_t) va_arg(args, hky_msec_t);
                if ((hky_msec_int_t) ms == -1) {
                    sign = 1;
                    i64 = -1;
                } else {
                    sign = 0;
                    ui64 = (uint64_t) ms;
                }
                break;

            case 'z':
                if (sign) {
                    i64 = (int64_t) va_arg(args, ssize_t);
                } else {
                    ui64 = (uint64_t) va_arg(args, size_t);
                }
                break;

            case 'i':
                if (sign) {
                    i64 = (int64_t) va_arg(args, hky_int_t);
                } else {
                    ui64 = (uint64_t) va_arg(args, hky_uint_t);
                }

                if (max_width) {
                    width = HKY_INT_T_LEN;
                }

                break;

            case 'd':
                if (sign) {
                    i64 = (int64_t) va_arg(args, int);
                } else {
                    ui64 = (uint64_t) va_arg(args, u_int);
                }
                break;

            case 'l':
                if (sign) {
                    i64 = (int64_t) va_arg(args, long);
                } else {
                    ui64 = (uint64_t) va_arg(args, u_long);
                }
                break;

            case 'D':
                if (sign) {
                    i64 = (int64_t) va_arg(args, int32_t);
                } else {
                    ui64 = (uint64_t) va_arg(args, uint32_t);
                }
                break;

            case 'L':
                if (sign) {
                    i64 = va_arg(args, int64_t);
                } else {
                    ui64 = va_arg(args, uint64_t);
                }
                break;

            case 'A':
                if (sign) {
                    i64 = (int64_t) va_arg(args, hky_atomic_int_t);
                } else {
                    ui64 = (uint64_t) va_arg(args, hky_atomic_uint_t);
                }

                if (max_width) {
                    width = HKY_ATOMIC_T_LEN;
                }

                break;

            case 'f':
                f = va_arg(args, double);

                if (f < 0) {
                    *buf++ = '-';
                    f = -f;
                }

                ui64 = (int64_t) f;
                frac = 0;

                if (frac_width) {

                    scale = 1;
                    for (n = frac_width; n; n--) {
                        scale *= 10;
                    }

                    frac = (uint64_t) ((f - (double) ui64) * scale + 0.5);

                    if (frac == scale) {
                        ui64++;
                        frac = 0;
                    }
                }

                buf = hky_sprintf_num(buf, last, ui64, zero, 0, width);

                if (frac_width) {
                    if (buf < last) {
                        *buf++ = '.';
                    }

                    buf = hky_sprintf_num(buf, last, frac, '0', 0, frac_width);
                }

                fmt++;

                continue;

#if !(HKY_WIN32)
            case 'r':
                i64 = (int64_t) va_arg(args, rlim_t);
                sign = 1;
                break;
#endif

            case 'p':
                ui64 = (uintptr_t) va_arg(args, void *);
                hex = 2;
                sign = 0;
                zero = '0';
                width = 2 * sizeof(void *);
                break;

            case 'c':
                d = va_arg(args, int);
                *buf++ = (hky_uchar) (d & 0xff);
                fmt++;

                continue;

            case 'Z':
                *buf++ = '\0';
                fmt++;

                continue;

            case 'N':
#if (HKY_WIN32)
                *buf++ = CR;
                if (buf < last) {
                    *buf++ = LF;
                }
#else
                *buf++ = LF;
#endif
                fmt++;

                continue;

            case '%':
                *buf++ = '%';
                fmt++;

                continue;

            default:
                *buf++ = *fmt++;

                continue;
            }

            if (sign) {
                if (i64 < 0) {
                    *buf++ = '-';
                    ui64 = (uint64_t) -i64;

                } else {
                    ui64 = (uint64_t) i64;
                }
            }

            buf = hky_sprintf_num(buf, last, ui64, zero, hex, width);

            fmt++;

        } else {
            *buf++ = *fmt++;
        }
    }

    return buf;
}
static hky_uchar *
hky_sprintf_num(hky_uchar *buf,hky_uchar *last,hky_uint64 ui64,hky_uchar zero,hky_uint_t hexadecimal,hky_uint_t width){
    hky_uchar         *p, temp[HKY_INT64_LEN + 1];
                       /*
                        * we need temp[NGX_INT64_LEN] only,
                        * but icc issues the warning
                        */
    size_t          len;
    uint32_t        ui32;
    static hky_uchar   hex[] = "0123456789abcdef";
    static hky_uchar   HEX[] = "0123456789ABCDEF";

    p = temp + HKY_INT64_LEN;

    if (hexadecimal == 0) {

        if (ui64 <= (uint64_t) HKY_MAX_UINT32_VALUE) {

            /*
             * To divide 64-bit numbers and to find remainders
             * on the x86 platform gcc and icc call the libc functions
             * [u]divdi3() and [u]moddi3(), they call another function
             * in its turn.  On FreeBSD it is the qdivrem() function,
             * its source code is about 170 lines of the code.
             * The glibc counterpart is about 150 lines of the code.
             *
             * For 32-bit numbers and some divisors gcc and icc use
             * a inlined multiplication and shifts.  For example,
             * unsigned "i32 / 10" is compiled to
             *
             *     (i32 * 0xCCCCCCCD) >> 35
             */

            ui32 = (uint32_t) ui64;

            do {
                *--p = (hky_uchar) (ui32 % 10 + '0');
            } while (ui32 /= 10);

        } else {
            do {
                *--p = (hky_uchar) (ui64 % 10 + '0');
            } while (ui64 /= 10);
        }

    } else if (hexadecimal == 1) {

        do {

            /* the "(uint32_t)" cast disables the BCC's warning */
            *--p = hex[(uint32_t) (ui64 & 0xf)];

        } while (ui64 >>= 4);

    } else { /* hexadecimal == 2 */

        do {

            /* the "(uint32_t)" cast disables the BCC's warning */
            *--p = HEX[(uint32_t) (ui64 & 0xf)];

        } while (ui64 >>= 4);
    }

    /* zero or space padding */

    len = (temp + HKY_INT64_LEN) - p;

    while (len++ < width && buf < last) {
        *buf++ = zero;
    }

    /* number safe copy */

    len = (temp + HKY_INT64_LEN) - p;

    if (buf + len > last) {
        len = last - buf;
    }

    return hky_cpymem(buf, p, len);
}

hky_int_t
hky_strcasecmp(hky_uchar *s1,hky_uchar *s2){
    hky_uint_t c1,c2;
    for(;;){
        c1=(hky_uint_t) *s1++;
        c2=(hky_uint_t) *s2++;

        c1=(c1>='A'&&c1<='Z')?(c1|0x20):c1;
        c2=(c2>='A'&&c2<='Z')?(c2|0x20):c2;

        if(c1==c2){
            if(c1){
                continue;
            }
            return 0;
        }
        return c1-c2;
    }
}
hky_int_t
hky_strncasecmp(hky_uchar *s1,hky_uchar *s2,size_t n){
    hky_uint_t c1,c2;
    while(n){
        c1=(hky_uint_t) *s1++;
        c2=(hky_uint_t) *s2++;

        c1=(c1>='A'&&c1<='Z')?(c1|0x20):c1;
        c2=(c2>='A'&&c2<='Z')?(c2|0x20):c2;

        if(c1==c2){
            if(c1){
                n--;continue;
            }
            return 0;
        }
        return c1-c2;
    }
    return 0;
}

hky_uchar *
hky_strnstr(hky_uchar *s1,hky_char *s2,size_t len){
    hky_uchar c1,c2;
    size_t n;

    c2=*(hky_uchar*)s2++;

    n=hky_strlen(s2);

    do{
        do{
            if(len--==0){
                return NULL;
            }
            c1=*s1++;
            if(c1==0){
                return NULL;
            }
        }while(c1!=c2);

        if(n>len){
            return NULL;
        }
    } while(hky_strncmp(s1,(hky_uchar*)s2,n)!=0);
    return --s1;
}

hky_uchar *
hky_strstrn(hky_uchar *s1,hky_char *s2,size_t n){
    hky_uchar c1,c2;
    c2=*(hky_uchar *)s2++;

    do{
        do{
            c1=*s1++;
            if(c1==0){
                return NULL;
            }
        }while(c1!=c2);
    }while(hky_strncmp(s1,(hky_uchar*)s2,n)!=0);
    return --s1;
}
hky_uchar *
hky_strcasestrn(hky_uchar *s1,hky_char *s2,size_t n){
    hky_uint_t c1,c2;
    c2=(hky_uint_t) *s2++;
    c2=(c2>='A'&&c2<='Z')?(c2|0x20) :c2;

    do{
        do{
            c1=(hky_uint_t) *s1++;
            if(c1==0){
                return NULL;
            }

            c1=(c1>='A'&&c1<='Z')?(c1|0x20):c1;
        }while(c1!=c2);
    }while(hky_strncasecmp(s1,(hky_uchar*)s2,n)!=0);

    return --s1;
}
hky_uchar *
hky_strlcasestrn(hky_uchar *s1,hky_uchar *last,hky_uchar *s2,size_t n){
    hky_uint_t c1,c2;

    c2=(hky_uint_t)*s2++;
    c2=(c2>='A'&&c2<='Z')?(c2|0x20):c2;
    last-=n;

    do{
        do{
                if(s1>=last){
                    return NULL;
                }
                c1=(hky_uint_t) *s1++;
                c1=(c1>='A'&&c1<='Z')?(c1|0x20):c1;
        }while(c1!=c2);
    }while(hky_strncasecmp(s1,s2,n)!=0);
    return --s1;
}

hky_int_t
hky_rstrncmp(hky_uchar *s1,hky_uchar *s2,size_t n){
    if(n==0){
        return 0;
    }
    n--;
    for(;;){
        if(s1[n]!=s2[n]){
            return s1[n]-s2[n];
        }
        if(n==0){
            return 0;
        }
        n--;
    }
}
hky_int_t
hky_rstrncasecmp(hky_uchar *s1,hky_uchar *s2,size_t n){
    hky_uchar c1,c2;
    if(n==0){
        return 0;
    }
    n--;
    for(;;){
        c1=s1[n];
        if(c1>='a'&&c1<='z'){
            c1-='a'-'A';
        }
        c2=s2[n];
        if(c2>='a'&&c2<='z'){
            c2-='a'-'A';
        }

        if(c1!=c2){
            return c1-c2;
        }
        if(n==0){
            return 0;
        }
        n--;
    }
}
hky_int_t
hky_memn2cmp(hky_uchar *s1,hky_uchar *s2,size_t n1,size_t n2){
    size_t n;
    hky_int_t m,z;
    if(n1<=n2){
        n=n1;
        z=-1;
    }else{
        n=n2;
        z=1;
    }
    m= hky_memcmp(s1,s2,n);
    if(m||n1==n2){
        return m;
    }
    return z;
}
hky_int_t
hky_dns_strcmp(hky_uchar *s1,hky_uchar *s2){
    hky_uint_t c1,c2;

    for(;;){
        c1=(hky_uint_t) *s1++;
        c2=(hky_uint_t) *s2++;

        c1 = (c1 >= 'A' && c1 <= 'Z') ? (c1 | 0x20) : c1;
        c2 = (c2 >= 'A' && c2 <= 'Z') ? (c2 | 0x20) : c2;

        if(c1==c2){
            if(c1){
                continue;
            }
            return 0;
        }

        c1=(c1=='.')?' ':c1;
        c2=(c2=='.')?' ':c2;
        return c1-c2;
    }
}
hky_int_t
hky_filename_cmp(hky_uchar *s1,hky_uchar *s2,size_t n){
    hky_uint_t c1,c2;

    while(n){
        c1=(hky_uint_t) *s1++;
        c2=(hky_uint_t) *s2++;

        #if (HKY_HAVE_CASELESS_FILESYSTEM)
            c1=tolower(c1);
            c2=tolower(c2);
        #endif
        if(c1==c2){
            if(c1){
                n--;
                continue;
            }
            return 0;
        }
        if(c1==0||c2==0){
            return c1-c2;
        }
        c1=(c1=='/')?0:c1;
        c2=(c2=='/')?0:c2;

        return c1-c2;
    }
    return 0;
}
hky_int_t
hky_atoi(hky_uchar *line,size_t n){
    hky_int_t value,cutoff,cutlim;
    if(n==0){
        return HKY_ERROR;
    }

    cutoff = HKY_MAX_INT_T_VALUE /10 ;
    cutlim=HKY_MAX_INT_T_VALUE %10 ;

    for(value=0;n--;line++){
        if(*line<'0'||*line>'9'){
            return HKY_ERROR;
        }
        if(value>=cutoff&& (value>cutoff||*line-'0'>cutlim)){
            return HKY_ERROR;
        }
        value=value*10+(*line-'0');
    }
    return value;
}
hky_int_t
hky_atofp(hky_uchar *line,size_t n,size_t point){
    hky_int_t value,cutoff,cutlim;
    hky_uint_t dot;

    if(n==0){
        return HKY_ERROR;
    }

    cutoff=HKY_MAX_INT_T_VALUE/10;
    cutlim=HKY_MAX_INT_T_VALUE%10;
    dot=0;
    for(value=0;n--;line++){
        if(point==0){
            return HKY_ERROR;
        }
        if(*line=='.'){
            if(dot){
                return HKY_ERROR;
            }
            dot=1;
            continue;
        }
        if(*line<'0'||*line>'9'){
            return HKY_ERROR;
        }
        if(value>=cutoff&&(value>cutoff||*line-'0'>cutlim)){
            return HKY_ERROR;
        }
        value=value*10+(*line-'0');
        point-=dot;
    }
    while(point--){
        if(value>cutoff){
            return HKY_ERROR;
        }
        value=value*10;
    }
    return value;
}
ssize_t
hky_atosz(hky_uchar *line,size_t n){
    ssize_t  value, cutoff, cutlim;

    if (n == 0) {
        return HKY_ERROR;
    }

    cutoff = HKY_MAX_SIZE_T_VALUE / 10;
    cutlim = HKY_MAX_SIZE_T_VALUE % 10;

    for (value = 0; n--; line++) {
        if (*line < '0' || *line > '9') {
            return HKY_ERROR;
        }

        if (value >= cutoff && (value > cutoff || *line - '0' > cutlim)) {
            return HKY_ERROR;
        }

        value = value * 10 + (*line - '0');
    }

    return value;
}
off_t
hky_atoof(hky_uchar *line,size_t n){
    off_t  value, cutoff, cutlim;

    if (n == 0) {
        return HKY_ERROR;
    }

    cutoff = HKY_MAX_OFF_T_VALUE / 10;
    cutlim = HKY_MAX_OFF_T_VALUE % 10;

    for (value = 0; n--; line++) {
        if (*line < '0' || *line > '9') {
            return HKY_ERROR;
        }

        if (value >= cutoff && (value > cutoff || *line - '0' > cutlim)) {
            return HKY_ERROR;
        }

        value = value * 10 + (*line - '0');
    }
    return value;
}
time_t
hky_atotm(hky_uchar *line,size_t n){
    time_t  value, cutoff, cutlim;

    if (n == 0) {
        return HKY_ERROR;
    }

    cutoff = HKY_MAX_TIME_T_VALUE / 10;
    cutlim = HKY_MAX_TIME_T_VALUE % 10;

    for (value = 0; n--; line++) {
        if (*line < '0' || *line > '9') {
            return HKY_ERROR;
        }

        if (value >= cutoff && (value > cutoff || *line - '0' > cutlim)) {
            return HKY_ERROR;
        }

        value = value * 10 + (*line - '0');
    }

    return value;
}
hky_int_t
hky_hextoi(hky_uchar *line,size_t n){
    hky_uchar     c, ch;
    hky_int_t  value, cutoff;

    if (n == 0) {
        return HKY_ERROR;
    }

    cutoff = HKY_MAX_INT_T_VALUE / 16;

    for (value = 0; n--; line++) {
        if (value > cutoff) {
            return HKY_ERROR;
        }

        ch = *line;

        if (ch >= '0' && ch <= '9') {
            value = value * 16 + (ch - '0');
            continue;
        }

        c = (hky_uchar) (ch | 0x20);

        if (c >= 'a' && c <= 'f') {
            value = value * 16 + (c - 'a' + 10);
            continue;
        }

        return HKY_ERROR;
    }

    return value;
}
hky_uchar *
hky_hex_dump(hky_uchar *dst,hky_uchar *src,size_t len){
    static hky_uchar  hex[] = "0123456789abcdef";

    while (len--) {
        *dst++ = hex[*src >> 4];
        *dst++ = hex[*src++ & 0xf];
    }

    return dst;
}
static void
hky_encode_base64_internal(hky_str_t *dst,hky_str_t *src,const hky_uchar *basis,hky_uint_t padding){
    hky_uchar  *d, *s;
    size_t          len;

    len = src->len;
    s = src->data;
    d = dst->data;

    while (len > 2) {
        *d++ = basis[(s[0] >> 2) & 0x3f];
        *d++ = basis[((s[0] & 3) << 4) | (s[1] >> 4)];
        *d++ = basis[((s[1] & 0x0f) << 2) | (s[2] >> 6)];
        *d++ = basis[s[2] & 0x3f];

        s += 3;
        len -= 3;
    }

    if (len) {
        *d++ = basis[(s[0] >> 2) & 0x3f];

        if (len == 1) {
            *d++ = basis[(s[0] & 3) << 4];
            if (padding) {
                *d++ = '=';
            }

        } else {
            *d++ = basis[((s[0] & 3) << 4) | (s[1] >> 4)];
            *d++ = basis[(s[1] & 0x0f) << 2];
        }

        if (padding) {
            *d++ = '=';
        }
    }

    dst->len = d - dst->data;
}
static hky_int_t
hky_decode_base64_internal(hky_str_t *dst,hky_str_t *src,const hky_uchar *basis){
    size_t          len;
    hky_uchar *d, *s;

    for (len = 0; len < src->len; len++) {
        if (src->data[len] == '=') {
            break;
        }

        if (basis[src->data[len]] == 77) {
            return HKY_ERROR;
        }
    }

    if (len % 4 == 1) {
        return HKY_ERROR;
    }

    s = src->data;
    d = dst->data;

    while (len > 3) {
        *d++ = (hky_uchar) (basis[s[0]] << 2 | basis[s[1]] >> 4);
        *d++ = (hky_uchar) (basis[s[1]] << 4 | basis[s[2]] >> 2);
        *d++ = (hky_uchar) (basis[s[2]] << 6 | basis[s[3]]);

        s += 4;
        len -= 4;
    }

    if (len > 1) {
        *d++ = (hky_uchar) (basis[s[0]] << 2 | basis[s[1]] >> 4);
    }

    if (len > 2) {
        *d++ = (hky_uchar) (basis[s[1]] << 4 | basis[s[2]] >> 2);
    }

    dst->len = d - dst->data;

    return HKY_OK;
}
void
hky_encode_base64(hky_str_t *dst,hky_str_t *src){
static hky_uchar   basis64[] =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    hky_encode_base64_internal(dst, src, basis64, 1);
}
void
hky_encode_base64url(hky_str_t *dst,hky_str_t *src){
static hky_uchar   basis64[] =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

   hky_encode_base64_internal(dst, src, basis64, 0);
}
hky_int_t
hky_decode_base64(hky_str_t *dst,hky_str_t *src){
static hky_uchar   basis64[] = {
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 62, 77, 77, 77, 63,
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 77, 77, 77, 77, 77, 77,
        77,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
        15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 77, 77, 77, 77, 77,
        77, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
        41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 77, 77, 77, 77, 77,

        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77
    };

    return hky_decode_base64_internal(dst, src, basis64);
}
hky_int_t
hky_decode_base64url(hky_str_t *dst,hky_str_t *src){
static hky_uchar   basis64[] = {
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 62, 77, 77,
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 77, 77, 77, 77, 77, 77,
        77,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
        15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 77, 77, 77, 77, 63,
        77, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
        41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 77, 77, 77, 77, 77,

        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77
    };

    return hky_decode_base64_internal(dst, src, basis64);
}
hky_uint32
hky_utf8_decode(hky_uchar **p,size_t n){
size_t    len;
    uint32_t  u, i, valid;
    u = **p;
    if (u >= 0xf0) {
        u &= 0x07;
        valid = 0xffff;
        len = 3;
    } else if (u >= 0xe0) {
        u &= 0x0f;
        valid = 0x7ff;
        len = 2;
    } else if (u >= 0xc2) {
        u &= 0x1f;
        valid = 0x7f;
        len = 1;

    } else {
        (*p)++;
        return 0xffffffff;
    }
    if (n - 1 < len) {
        return 0xfffffffe;
    }
    (*p)++;
    while (len) {
        i = *(*p)++;
        if (i < 0x80) {
            return 0xffffffff;
        }
        u = (u << 6) | (i & 0x3f);
        len--;
    }
    if (u > valid) {
        return u;
    }
    return 0xffffffff;
}
size_t
hky_utf8_length(hky_uchar *p,size_t n){
    hky_uchar  c, *last;
    size_t  len;
    last = p + n;
    for (len = 0; p < last; len++) {
        c = *p;
        if (c < 0x80) {
            p++;
            continue;
        }
        if (hky_utf8_decode(&p, n) > 0x10ffff) {
            return n;
        }
    }
    return len;
}
hky_uchar *
hky_utf8_cpystrn(hky_uchar *dst,hky_uchar *src, size_t n,size_t len){
    hky_uchar  c, *next;
    if (n == 0) {
        return dst;
    }
    while (--n) {
        c = *src;
        *dst = c;
        if (c < 0x80) {
            if (c != '\0') {
                dst++;
                src++;
                len--;
                continue;
            }
            return dst;
        }
        next = src;
        if (hky_utf8_decode(&next, len) > 0x10ffff) {
            /* invalid UTF-8 */
            break;
        }
        while (src < next) {
            *dst++ = *src++;
            len--;
        }
    }
    *dst = '\0';
    return dst;
}
uintptr_t
hky_escape_uri(hky_uchar *dst,hky_uchar *src, size_t size,hky_uint_t type){
    hky_uint_t      n;
    uint32_t       *escape;
    static hky_uchar   hex[] = "0123456789ABCDEF";

                    /* " ", "#", "%", "?", %00-%1F, %7F-%FF */

    static uint32_t   uri[] = {
        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */

                    /* ?>=< ;:98 7654 3210  /.-, +*)( '&%$ #"!  */
        0x80000029, /* 1000 0000 0000 0000  0000 0000 0010 1001 */

                    /* _^]\ [ZYX WVUT SRQP  ONML KJIH GFED CBA@ */
        0x00000000, /* 0000 0000 0000 0000  0000 0000 0000 0000 */

                    /*  ~}| {zyx wvut srqp  onml kjih gfed cba` */
        0x80000000, /* 1000 0000 0000 0000  0000 0000 0000 0000 */

        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
        0xffffffff  /* 1111 1111 1111 1111  1111 1111 1111 1111 */
    };

                    /* " ", "#", "%", "&", "+", "?", %00-%1F, %7F-%FF */

    static uint32_t   args[] = {
        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */

                    /* ?>=< ;:98 7654 3210  /.-, +*)( '&%$ #"!  */
        0x88000869, /* 1000 1000 0000 0000  0000 1000 0110 1001 */

                    /* _^]\ [ZYX WVUT SRQP  ONML KJIH GFED CBA@ */
        0x00000000, /* 0000 0000 0000 0000  0000 0000 0000 0000 */

                    /*  ~}| {zyx wvut srqp  onml kjih gfed cba` */
        0x80000000, /* 1000 0000 0000 0000  0000 0000 0000 0000 */

        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
        0xffffffff  /* 1111 1111 1111 1111  1111 1111 1111 1111 */
    };

                    /* not ALPHA, DIGIT, "-", ".", "_", "~" */

    static uint32_t   uri_component[] = {
        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */

                    /* ?>=< ;:98 7654 3210  /.-, +*)( '&%$ #"!  */
        0xfc009fff, /* 1111 1100 0000 0000  1001 1111 1111 1111 */

                    /* _^]\ [ZYX WVUT SRQP  ONML KJIH GFED CBA@ */
        0x78000001, /* 0111 1000 0000 0000  0000 0000 0000 0001 */

                    /*  ~}| {zyx wvut srqp  onml kjih gfed cba` */
        0xb8000001, /* 1011 1000 0000 0000  0000 0000 0000 0001 */

        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
        0xffffffff  /* 1111 1111 1111 1111  1111 1111 1111 1111 */
    };

                    /* " ", "#", """, "%", "'", %00-%1F, %7F-%FF */

    static uint32_t   html[] = {
        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */

                    /* ?>=< ;:98 7654 3210  /.-, +*)( '&%$ #"!  */
        0x000000ad, /* 0000 0000 0000 0000  0000 0000 1010 1101 */

                    /* _^]\ [ZYX WVUT SRQP  ONML KJIH GFED CBA@ */
        0x00000000, /* 0000 0000 0000 0000  0000 0000 0000 0000 */

                    /*  ~}| {zyx wvut srqp  onml kjih gfed cba` */
        0x80000000, /* 1000 0000 0000 0000  0000 0000 0000 0000 */

        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
        0xffffffff  /* 1111 1111 1111 1111  1111 1111 1111 1111 */
    };

                    /* " ", """, "%", "'", %00-%1F, %7F-%FF */

    static uint32_t   refresh[] = {
        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */

                    /* ?>=< ;:98 7654 3210  /.-, +*)( '&%$ #"!  */
        0x00000085, /* 0000 0000 0000 0000  0000 0000 1000 0101 */

                    /* _^]\ [ZYX WVUT SRQP  ONML KJIH GFED CBA@ */
        0x00000000, /* 0000 0000 0000 0000  0000 0000 0000 0000 */

                    /*  ~}| {zyx wvut srqp  onml kjih gfed cba` */
        0x80000000, /* 1000 0000 0000 0000  0000 0000 0000 0000 */

        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
        0xffffffff  /* 1111 1111 1111 1111  1111 1111 1111 1111 */
    };

                    /* " ", "%", %00-%1F */

    static uint32_t   memcached[] = {
        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */

                    /* ?>=< ;:98 7654 3210  /.-, +*)( '&%$ #"!  */
        0x00000021, /* 0000 0000 0000 0000  0000 0000 0010 0001 */

                    /* _^]\ [ZYX WVUT SRQP  ONML KJIH GFED CBA@ */
        0x00000000, /* 0000 0000 0000 0000  0000 0000 0000 0000 */

                    /*  ~}| {zyx wvut srqp  onml kjih gfed cba` */
        0x00000000, /* 0000 0000 0000 0000  0000 0000 0000 0000 */

        0x00000000, /* 0000 0000 0000 0000  0000 0000 0000 0000 */
        0x00000000, /* 0000 0000 0000 0000  0000 0000 0000 0000 */
        0x00000000, /* 0000 0000 0000 0000  0000 0000 0000 0000 */
        0x00000000, /* 0000 0000 0000 0000  0000 0000 0000 0000 */
    };

                    /* mail_auth is the same as memcached */

    static uint32_t  *map[] =
        { uri, args, uri_component, html, refresh, memcached, memcached };


    escape = map[type];

    if (dst == NULL) {

        /* find the number of the characters to be escaped */

        n = 0;

        while (size) {
            if (escape[*src >> 5] & (1U << (*src & 0x1f))) {
                n++;
            }
            src++;
            size--;
        }

        return (uintptr_t) n;
    }

    while (size) {
        if (escape[*src >> 5] & (1U << (*src & 0x1f))) {
            *dst++ = '%';
            *dst++ = hex[*src >> 4];
            *dst++ = hex[*src & 0xf];
            src++;

        } else {
            *dst++ = *src++;
        }
        size--;
    }

    return (uintptr_t) dst;
}
void
hky_unescape_uri(hky_uchar **dst,hky_uchar **src,size_t size,hky_uint_t type){
 hky_uchar  *d, *s, ch, c, decoded;
    enum {
        sw_usual = 0,
        sw_quoted,
        sw_quoted_second
    } state;

    d = *dst;
    s = *src;

    state = 0;
    decoded = 0;

    while (size--) {

        ch = *s++;

        switch (state) {
        case sw_usual:
            if (ch == '?'
                && (type & (HKY_UNESCAPE_URI|HKY_UNESCAPE_REDIRECT)))
            {
                *d++ = ch;
                goto done;
            }

            if (ch == '%') {
                state = sw_quoted;
                break;
            }

            *d++ = ch;
            break;

        case sw_quoted:

            if (ch >= '0' && ch <= '9') {
                decoded = (hky_uchar) (ch - '0');
                state = sw_quoted_second;
                break;
            }

            c = (hky_uchar) (ch | 0x20);
            if (c >= 'a' && c <= 'f') {
                decoded = (hky_uchar) (c - 'a' + 10);
                state = sw_quoted_second;
                break;
            }

            /* the invalid quoted character */

            state = sw_usual;

            *d++ = ch;

            break;

        case sw_quoted_second:

            state = sw_usual;

            if (ch >= '0' && ch <= '9') {
                ch = (hky_uchar) ((decoded << 4) + (ch - '0'));

                if (type & HKY_UNESCAPE_REDIRECT) {
                    if (ch > '%' && ch < 0x7f) {
                        *d++ = ch;
                        break;
                    }

                    *d++ = '%'; *d++ = *(s - 2); *d++ = *(s - 1);

                    break;
                }

                *d++ = ch;

                break;
            }

            c = (hky_uchar) (ch | 0x20);
            if (c >= 'a' && c <= 'f') {
                ch = (hky_uchar) ((decoded << 4) + (c - 'a') + 10);

                if (type & HKY_UNESCAPE_URI) {
                    if (ch == '?') {
                        *d++ = ch;
                        goto done;
                    }

                    *d++ = ch;
                    break;
                }

                if (type & HKY_UNESCAPE_REDIRECT) {
                    if (ch == '?') {
                        *d++ = ch;
                        goto done;
                    }

                    if (ch > '%' && ch < 0x7f) {
                        *d++ = ch;
                        break;
                    }

                    *d++ = '%'; *d++ = *(s - 2); *d++ = *(s - 1);
                    break;
                }

                *d++ = ch;

                break;
            }
            /* the invalid quoted character */
            break;
        }
    }
done:
    *dst = d;
    *src = s;
}
uintptr_t
hky_escape_html(hky_uchar *dst,hky_uchar *src, size_t  size){
   hky_uchar      ch;
    hky_uint_t  len;

    if (dst == NULL) {

        len = 0;

        while (size) {
            switch (*src++) {

            case '<':
                len += sizeof("&lt;") - 2;
                break;

            case '>':
                len += sizeof("&gt;") - 2;
                break;

            case '&':
                len += sizeof("&amp;") - 2;
                break;

            case '"':
                len += sizeof("&quot;") - 2;
                break;

            default:
                break;
            }
            size--;
        }

        return (uintptr_t) len;
    }

    while (size) {
        ch = *src++;

        switch (ch) {

        case '<':
            *dst++ = '&'; *dst++ = 'l'; *dst++ = 't'; *dst++ = ';';
            break;

        case '>':
            *dst++ = '&'; *dst++ = 'g'; *dst++ = 't'; *dst++ = ';';
            break;

        case '&':
            *dst++ = '&'; *dst++ = 'a'; *dst++ = 'm'; *dst++ = 'p';
            *dst++ = ';';
            break;

        case '"':
            *dst++ = '&'; *dst++ = 'q'; *dst++ = 'u'; *dst++ = 'o';
            *dst++ = 't'; *dst++ = ';';
            break;

        default:
            *dst++ = ch;
            break;
        }
        size--;
    }

    return (uintptr_t) dst;
}
uintptr_t
hky_escape_json(hky_char *dst,hky_uchar *src,size_t size){
  hky_uchar      ch;
    hky_uint_t  len;

    if (dst == NULL) {
        len = 0;

        while (size) {
            ch = *src++;

            if (ch == '\\' || ch == '"') {
                len++;

            } else if (ch <= 0x1f) {

                switch (ch) {
                case '\n':
                case '\r':
                case '\t':
                case '\b':
                case '\f':
                    len++;
                    break;

                default:
                    len += sizeof("\\u001F") - 2;
                }
            }

            size--;
        }

        return (uintptr_t) len;
    }

    while (size) {
        ch = *src++;

        if (ch > 0x1f) {

            if (ch == '\\' || ch == '"') {
                *dst++ = '\\';
            }

            *dst++ = ch;

        } else {
            *dst++ = '\\';

            switch (ch) {
            case '\n':
                *dst++ = 'n';
                break;

            case '\r':
                *dst++ = 'r';
                break;

            case '\t':
                *dst++ = 't';
                break;

            case '\b':
                *dst++ = 'b';
                break;

            case '\f':
                *dst++ = 'f';
                break;

            default:
                *dst++ = 'u'; *dst++ = '0'; *dst++ = '0';
                *dst++ = '0' + (ch >> 4);

                ch &= 0xf;

                *dst++ = (ch < 10) ? ('0' + ch) : ('A' + ch - 10);
            }
        }

        size--;
    }

    return (uintptr_t) dst;
}
void
hky_str_rbtree_insert_value(hky_rbtree_node_t *temp,hky_rbtree_node_t *node,hky_rbtree_node_t *sentinel){
    hky_str_node_t *n,*t;
    hky_rbtree_node_t **p;
    for(;;){
        n=(hky_str_node_t*) node;
        t=(hky_str_node_t*) temp;
        if(node->key!=temp->key){
            p=(node->key<temp->key) ?&temp->left:&temp->right;
        }else if(n->str.len!=t->str.len){
            p=(n->str.len<t->str.len) ?&temp->left:&temp->right;
        }else{
            p=(hky_memcmp(n->str.data,t->str.data,n->str.len)<0)
                ?&temp->left:&temp->right;
        }
        if(*p==sentinel){
            break;
        }
        temp=*p;
    }
    *p=node;
    node->parent=temp;
    node->left=sentinel;
    node->right=sentinel;
    hky_rbt_red(node);
}
hky_str_node_t *
hky_str_rbtree_lookup(hky_rbtree_t *rbtree,hky_str_t *val,uint32_t hash){
    hky_int_t rc;
    hky_str_node_t *n;
    hky_rbtree_node_t *node,*sentinel;

    node=rbtree->root;
    sentinel=rbtree->sentinel;

    while(node!=sentinel){
        n=(hky_str_node_t*)node;
        if(hash!=node->key){
            node=(hash<node->key)?node->left:node->right;
            continue;
        }
        if(val->len!=n->str.len){
            node=(val->len<n->str.len)?node->left:node->right;
            continue;
        }
        rc=hky_memcmp(val->data,n->str.data,val->len);
        if(rc<0){
            node=node->left;
            continue;
        }
        if(rc>0){
            node=node->right;
            continue;
        }
        return n;
    }
    return NULL;
}
void
hky_sort(void *base,size_t n,size_t size, hky_int_t (*cmp)(const void *,const void*)){

    hky_uchar  *p1, *p2, *p;
    p=NULL;
    //p = hky_alloc(size, hky_cycle->log);
    if (p == NULL) {
        return;
    }

    for (p1 = (hky_uchar *) base + size;
         p1 < (hky_uchar *) base + n * size;
         p1 += size)
    {
        hky_memcpy(p, p1, size);

        for (p2 = p1;
             p2 > (hky_uchar *) base && cmp(p2 - size, p) > 0;
             p2 -= size)
        {
            hky_memcpy(p2, p2 - size, size);
        }

        hky_memcpy(p2, p, size);
    }

    //hky_free(p);
}

/*
* ???
*/
#if (HKY_MEMCPY_LIMIT)
void *
hky_memcpy(void *dst,const void *src,size_t n){
    if(n>HKY_MEMCPY_LIMIT){

    }
    return memcpy(dst,src,n);
}
#endif
