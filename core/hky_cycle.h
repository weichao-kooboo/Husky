#ifndef HKY_CYCLE_H_INCLUDED
#define HKY_CYCLE_H_INCLUDED

#include "hky_config.h"
#include "hky_core.h"

#ifndef HKY_CYCLE_POOL_SIZE
#define HKY_CYCLE_POOL_SIZE	HKY_DEFAULT_POOL_SIZE
#endif // !HKY_CYCLE_POOL_SIZE

typedef struct hky_shm_zone_s hky_shm_zone_t;
typedef hky_int_t(*hky_shm_zone_init_pt)(hky_shm_zone_t *zone, void *data);

struct hky_shm_zone_s {
	void *data;
	hky_shm_t shm;
	hky_shm_zone_init_pt init;
	void *tag;
	void *sync;
	hky_uint_t noreuse;
};

struct hky_cycle_s{
	void ****conf_ctx;
    hky_pool_t *pool;

    hky_log_t *log;
    hky_log_t new_log;

    hky_uint_t log_use_stderr;

    hky_list_t open_files;
    hky_list_t shared_memory;

	hky_queue_t reusable_connections_queue;
	hky_uint_t   reusable_connections_n;

	//监听套接字数组
	hky_array_t listening;
	hky_array_t paths;

	hky_array_t config_dump;
	//dump的rbtree
	hky_rbtree_t config_dump_rbtree;
	//dump的rbtree节点
	hky_rbtree_node_t config_dump_sentinel;
	//保存老的cycle结构
	hky_cycle_t *old_cycle;
	//记录配置路径
    hky_str_t conf_file;
    hky_str_t conf_param;
	//记录前缀路径
    hky_str_t conf_prefix;
	//记录前缀路径
    hky_str_t prefix;
};

typedef struct {
	hky_flag_t daemon;
}hky_core_conf_t;

hky_cycle_t *hky_init_cycle(hky_cycle_t *old_cycle);

extern volatile hky_cycle_t *hky_cycle;
extern hky_uint_t             hky_test_config;
extern hky_uint_t             hky_dump_config;
extern hky_uint_t             hky_quiet_mode;

#endif // HKY_CYCLE_H_INCLUDED
