#ifndef HKY_BUF_H_INCLUDED
#define HKY_BUF_H_INCLUDED

#include "hky_config.h"
#include "hky_core.h"

typedef  void * hky_buf_tag_t;

typedef struct hky_buf_s hky_buf_t;

struct hky_buf_s{
    hky_uchar *pos;
    hky_uchar *last;
    off_t file_pos;
    off_t file_last;

    hky_uchar *start;
    hky_uchar *end;
    hky_buf_tag_t tag;
    hky_file_t *file;
    hky_buf_t *shadow;

    unsigned temporary:1;

    unsigned memory:1;

    unsigned mmap:1;
    unsigned recycled:1;
    unsigned in_file:1;
    unsigned flush:1;
    unsigned sync:1;
    unsigned last_buf:1;
    unsigned last_in_chain:1;
    unsigned last_shadow:1;
    unsigned temp_file:1;
    int num;
};

struct hky_chain_s{
    hky_buf_t *buf;
    hky_chain_t *next;
};

#endif // HKY_BUF_H_INCLUDED
