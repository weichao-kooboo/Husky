#include "hky_config.h"
#include "hky_core.h"

hky_buf_t *
hky_create_temp_buf(hky_pool_t *pool,size_t size){
    hky_buf_t *b;
    b=hky_calloc_buf(pool);
    if (NULL==b) {
        return NULL;
    }
    
    b->start=hky_palloc(pool, size);
    if (b->start==NULL) {
        return NULL;
    }
    
    b->pos=b->start;
    b->last=b->start;
    b->end=b->last+size;
    b->temporary=1;
    
    return b;
}

hky_chain_t *
hky_chain_update_sent(hky_chain_t *in, off_t sent) {
	off_t size;

	for (; in; in = in->next) {
		if (hky_buf_special(in->buf)) {
			continue;
		}
		if (sent == 0) {
			break;
		}
		size = hky_buf_size(in->buf);

		if (sent >= size) {
			sent -= size;
			if (hky_buf_in_memory(in->buf)) {
				in->buf->pos = in->buf->last;
			}
			if (in->buf->in_file) {
				in->buf->file_pos = in->buf->file_last;
			}
			continue;
		}
		if (hky_buf_in_memory(in->buf)) {
			in->buf->pos += (size_t)sent;
		}
		if (in->buf->in_file) {
			in->buf->file_pos += sent;
		}
		break;
	}
	return in;
}