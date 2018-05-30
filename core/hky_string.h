#ifndef HKY_STRING_H_INCLUDED
#define HKY_STRING_H_INCLUDED

#include "hky_config.h"
#include "hky_core.h"
/*标准字符串结构*/
typedef struct{
    //length
    size_t len;
    //data
    hky_uchar *data;
} hky_str_t;
/*键值对结构*/
typedef struct{
    hky_str_t key;
    hky_str_t value;
} hky_keyval_t;

typedef struct{
    hky_uint len:28;
    hky_uint valid:1;
    hky_uint no_cacheable:1;
    hky_uint not_found:1;
    hky_uint escape:1;
    hky_uchar *data;
} hky_variable_value_t;

#define hky_string(str) { sizeof(str)-1,(hky_uchar*)str}
#define hky_null_string {0,NULL}
#define hky_str_set(str,text)                                                       \
    (str)->len=sizeof(text)-1;(str)->data=(hky_uchar*)text
#define hky_str_null(str) (str)->len=0;(str)->data=NULL

#define hky_tolower(c) (hky_uchar)((c>='A'&&c<='Z')?(c|0x20):c)
#define hky_toupper(c) (hky_uchar)((c>='a'&&c<='z')?(c&~0x20):c)
/*
* 把src中字符变成小写放到dst中
* dst:目标指针
* src:源指针
* n:转换的长度
*/
void hky_strlow(hky_uchar *dst,hky_uchar *src,size_t n);

#define hky_strncmp(s1,s2,n) strncmp((const hky_char*)s1,(const hky_char*)s2,n)

#define hky_strcmp(s1,s2) strcmp((const hky_char*)s1,(const hky_char*)s2)

#define hky_strstr(s1,s2) strstr((const hky_char*)s1,(const hky_char*)s2)
#define hky_strlen(s) strlen((const hky_char*)s)

#define hky_strchr(s1,c) strchr((const hky_char*)s1,(hky_int*)c)

static hky_inline hky_uchar* hky_strlchr(hky_uchar *p,hky_uchar *last,hky_char c){
    while(p<last){
        if(*p==c){
            return p;
        }
        p++;
    }
    return NULL;
}

#define hky_memzero(buf,n) (void)memset(buf,0,n)
#define hky_memset(buf,c,n) (void)memset(buf,c,n)

#if(HKY_MEMCPY_LIMIT)
void *hky_memcpy(void *dst,const void *src,size_t n);
#define hky_cpymem(dst,src,n) (((hky_uchar*)hky_memcpy(dst,src,n))+(n))
#else
#define hky_memcpy(dst,src,n) (void)memcpy(dst,src,n)
#define hky_cpymem(dst,src,n) (((hky_uchar*)memcpy(dst,src,n))+(n))

#endif

#if ( __INTEL_COMPILER >= 800 )

static hky_inline hky_uchar *
hky_copy(hky_uchar *dst, hky_uchar *src, size_t len)
{
    if (len < 17) {

        while (len) {
            *dst++ = *src++;
            len--;
        }

        return dst;

    } else {
        return hky_cpymem(dst, src, len);
    }
}
#else
#define hky_copy                  hky_cpymem
#endif

#define hky_memmove(dst,src,n) (void)memmove(dst,src,n)
#define hky_movemem(dst,src,n) (((hky_uchar*)memmove(dst,src,n))+(n))

#define hky_memcmp(s1,s2,n) memcmp((const hky_char*)s1,(const hky_char*)s2,n)
/*
*    根据长度n复制字符串
*   dst:目标串
*  src:源串
*  n:长度
    return:如果源串长度小于n,直接返回
*/
hky_uchar *hky_cpystrn(hky_uchar *dst,hky_uchar *src,size_t n);
/*
*   动态分配内存,创建字符串
*    pool:内存池
*   src:字符串
*    return:新串地址
*/
hky_uchar *hky_pstrdup(hky_pool_t *pool,hky_str_t *src);

/*
 *   begin:
 *   格式化显示字符串,格式化的内容包括,自定义数据类型
 *    支持类型:
 *    %[0][width][x][X]O        off_t
 *    %[0][width]T              time_t
 *    %[0][width][u][x|X]z      ssize_t/size_t
 *    %[0][width][u][x|X]d      int/u_int
 *    %[0][width][u][x|X]l      long
 *    %[0][width|m][u][x|X]i    ngx_int_t/ngx_uint_t
 *    %[0][width][u][x|X]D      int32_t/uint32_t
 *    %[0][width][u][x|X]L      int64_t/uint64_t
 *    %[0][width|m][u][x|X]A    ngx_atomic_int_t/ngx_atomic_uint_t
 *    %[0][width][.width]f      double, max valid number fits to %18.15f
 *    %P                        ngx_pid_t
 *    %M                        ngx_msec_t
 *    %r                        rlim_t
 *    %p                        void *
 *    %V                        ngx_str_t *
 *    %v                        ngx_variable_value_t *
 *    %s                        null-terminated string
 *    %*s                       length and string
 *    %Z                        '\0'
 *    %N                        '\n'
 *    %c                        char
 *    %%                        %
 *
 *  reserved:
 *    %t                        ptrdiff_t
 *    %S                        null-terminated wchar string
 *    %C                        wchar
*/
hky_uchar *hky_cdecl hky_sprintf(hky_uchar *buf,const char *fmt, ...);
hky_uchar *hky_cdecl hky_snprintf(hky_uchar *buf,size_t max, const char *fmt,...);
hky_uchar *hky_cdecl hky_slprintf(hky_uchar *buf,hky_uchar *last,const char *fmt, ...);

hky_uchar *hky_vslprintf(hky_uchar *buf,hky_uchar *last,const char *fmt,va_list args);
#define hky_vsnprintf(buf,max,fmt,args) hky_vslprintf(buf,buf+(max),fmt,args)
/*end*/

/*
*比较字符串大小,根据每个串内字符转换成大写逐个比较,遇到不相同的返回字符ASCII码差值
*s1:字符串1
*s2:字符串2
*return: 0:字符相等. 大于0:s1>s2 . 小于0:s1<s2
*/
hky_int_t hky_strcasecmp(hky_uchar *s1,hky_uchar *s2);
/*
*比较字符串大小,根据每个串内字符转换成大写逐个比较,遇到不相同的返回字符ASCII码差值
*s1:字符串1
*s2:字符串2
*n: 匹配的长度
*return: 0:字符相等. 大于0:s1>s2 . 小于0:s1<s2
*/
hky_int_t hky_strncasecmp(hky_uchar *s1,hky_uchar *s2,size_t n);
/*
* 匹配子串,
* s1:源串
* s2:子串
* len:需要匹配的长度
* return:返回s1中子串的头指针
*/
hky_uchar *hky_strnstr(hky_uchar *s1,hky_char *s2,size_t len);
/*
* 匹配子串,
* s1:源串
* s2:子串
* n:需要匹配的长度,必须是s2长度-1
* return:返回s1中子串的头指针
*/
hky_uchar *hky_strstrn(hky_uchar *s1,hky_char *s2,size_t n);
/*
*匹配子串,不区分大小写
* s1:源串
* s2:子串
* n:需要匹配的长度
* return:返回s1中子串的头指针
*/
hky_uchar *hky_strcasestrn(hky_uchar *s1,hky_char *s2,size_t n);
/*
*  ???
*/
hky_uchar *hky_strlcasestrn(hky_uchar *s1,hky_uchar *last,hky_uchar *s2,size_t n);

/*
* 反向匹配字符串,从最后一位开始匹配
* s1:源串,
* s2:匹配串
* n:开始匹配的长度
* return: 如果相等返回0.不想等返回字母大小之差
*/
hky_int_t hky_rstrncmp(hky_uchar *s1,hky_uchar *s2,size_t n);
/*
* 反向匹配字符串,从最后一位开始匹配,不区分大小写
* s1:源串,
* s2:匹配串
* n:开始匹配的长度
* return: 如果相等返回0.不想等返回字母大小之差
*/
hky_int_t hky_rstrncasecmp(hky_uchar *s1,hky_uchar *s2,size_t n);
/*
* 使用memcmp匹配字符串
* s1:源串
* s2:匹配串
* n1:长度1
* n2:长度2
* return: s1 > s2 返回1,相等返回0,s1<s2 返回-1
*/
hky_int_t hky_memn2cmp(hky_uchar *s1,hky_uchar *s2,size_t n1,size_t n2);
/*
* 匹配两个dns地址
* s1:源串
* s2:匹配串
* return: s1 > s2 返回1,相等返回0,s1<s2 返回-1
*/
hky_int_t hky_dns_strcmp(hky_uchar *s1,hky_uchar *s2);
/*
* 匹配两个文件地址
* s1:源串
* s2:匹配串
* return: s1 > s2 返回1,相等返回0,s1<s2 返回-1
*/
hky_int_t hky_filename_cmp(hky_uchar *s1,hky_uchar *s2,size_t n);
/*
* 端口号转为int
* line:字符串
* n:字符串长度
* return:返回int型端口号
*/
hky_int_t hky_atoi(hky_uchar *line,size_t n);
/*
* 端口号转为int,字符串可以是小数,hky_atofp("10.5", 4, 2) returns 1050
* line:字符串
* n:字符串长度
* point:小数部分长度
* return:返回int型端口号
*/
hky_int_t hky_atofp(hky_uchar *line,size_t n,size_t point);
/*
* 端口号转为ssize
* line:字符串
* n:字符串长度
* return:返回ssize型端口号
*/
ssize_t hky_atosz(hky_uchar *line,size_t n);
/*
* 端口号转为off_t
* line:字符串
* n:字符串长度
* return:返回off_t型端口号
*/
off_t hky_atoof(hky_uchar *line,size_t n);
/*
* 端口号转为time_t
* line:字符串
* n:字符串长度
* return:返回time_t型端口号
*/
time_t hky_atotm(hky_uchar *line,size_t n);
/*
* 16进制转为十进制
* line:16进制字符串
* n:字符串长度
* return: 10进制数
*/
hky_int_t hky_hextoi(hky_uchar *line,size_t n);
/*
* ???
*/
hky_uchar *hky_hex_dump(hky_uchar *dst,hky_uchar *src,size_t len);

#define hky_base64_encoded_length(len) (((len+2)/3)*4)
#define hky_base64_decoded_length(len) (((len+3)/4)*3)
/*
* base64编码字符串
* dst:目标字符串
* src:源字符串
*/
void hky_encode_base64(hky_str_t *dst,hky_str_t *src);
/*
* base64编码url字符串
* dst:目标字符串
* src:源字符串
*/
void hky_encode_base64url(hky_str_t *dst,hky_str_t *src);
/*
* base64解码字符串
* dst:目标字符串
* src:源字符串
* return:0:成功
*/
hky_int_t hky_decode_base64(hky_str_t *dst,hky_str_t *src);
/*
* base64解码url字符串
* dst:目标字符串
* src:源字符串
* return:0:成功
*/
hky_int_t hky_decode_base64url(hky_str_t *dst,hky_str_t *src);
/*
* ???
*/
hky_uint32 hky_utf8_decode(hky_uchar **p,size_t n);
/*
* ???
*/
size_t hky_utf8_length(hky_uchar *p,size_t n);
/*
* ???
*/
hky_uchar *hky_utf8_cpystrn(hky_uchar *dst,hky_uchar *src, size_t n,size_t len);

#define HKY_ESCAPE_URI            0
#define HKY_ESCAPE_ARGS           1
#define HKY_ESCAPE_URI_COMPONENT  2
#define HKY_ESCAPE_HTML           3
#define HKY_ESCAPE_REFRESH        4
#define HKY_ESCAPE_MEMCACHED      5
#define HKY_ESCAPE_MAIL_AUTH      6

#define HKY_UNESCAPE_URI       1
#define HKY_UNESCAPE_REDIRECT  2

uintptr_t hky_escape_uri(hky_uchar *dst,hky_uchar *src, size_t size,hky_uint_t type);
void hky_unescape_uri(hky_uchar **dst,hky_uchar **src,size_t size,hky_uint_t type);
uintptr_t hky_escape_html(hky_uchar *dst,hky_uchar *src, size_t  size);
uintptr_t hky_escape_json(hky_char *dst,hky_uchar *src,size_t size);

//rbtree
typedef struct{
    hky_rbtree_node_t node;
    hky_str_t str;
} hky_str_node_t;
/*
* rbtree插入节点,节点类型是字符串包裹tree结构,
* temp:tree的节点,一般是root
* node:待插入的节点
* sentinel: 哨兵节点,标准rbtree的是null
*/
void hky_str_rbtree_insert_value(hky_rbtree_node_t *temp,hky_rbtree_node_t *node,hky_rbtree_node_t *sentinel);
/*
* rbtree查找节点是否存在
* rbtree:rb树
* name:需要查找的节点
* hash:匹配的键值
*/
hky_str_node_t *hky_str_rbtree_lookup(hky_rbtree_t *rbtree,hky_str_t *name,uint32_t hash);
/*
* ???
*/
void hky_sort(void *base,size_t n,size_t size, hky_int_t (*cmp)(const void *,const void*));
#define hky_qsort qsort

#define hky_value_helper(n) #n
#define hky_value(n) hky_value_helper(n)

#endif // HKY_STRING_H_INCLUDED
