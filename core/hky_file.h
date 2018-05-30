#ifndef HKY_FILE_H_INCLUDED
#define HKY_FILE_H_INCLUDED

#include "hky_config.h"
#include "hky_core.h"

struct hky_file_s{
    hky_fd_t fd;
    hky_str_t name;
};

#define HKY_MAX_PATH_LEVEL 3

typedef hky_msec_t(*hky_path_manager_pt)(void *data);
typedef hky_msec_t(*hky_path_purget_pt)(void *data);
typedef void(*hky_path_loader_pt)(void *data);

typedef struct {
	hky_str_t name;
	size_t len;
	size_t level[HKY_MAX_PATH_LEVEL];

	hky_path_manager_pt manager;
	hky_path_purget_pt purger;
	hky_path_loader_pt loader;
	void *data;
	hky_uchar *conf_file;
	hky_uint_t line;
}hky_path_t;

hky_int_t hky_get_full_name(hky_pool_t *pool,hky_str_t *prefix,hky_str_t *name);

#endif // HKY_FILE_H_INCLUDED
