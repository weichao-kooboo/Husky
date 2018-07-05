#pragma once

#include "../core/hky_config.h"
#include "../core/hky_core.h"
#include "../event/hky_event.h"

hky_chain_t *
hky_writev_chain(hky_connection_t *c, hky_chain_t *in, off_t limit) {

}

hky_chain_t *
hky_output_chain_to_iovec(hky_iovec_t *vec, hky_chain_t *in,
	size_t limit, hky_log_t *log) {

}

ssize_t 
hky_writev(hky_connection_t *c, hky_iovec_t *vec) {

}