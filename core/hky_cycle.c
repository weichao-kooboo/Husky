
#include "hky_config.h"
#include "hky_core.h"

hky_cycle_t *
hky_init_cycle(hky_cycle_t *old_cycle) {
	void *rv;
	char **senv;
	hky_uint_t i, n;
	hky_log_t *log;
	hky_time_t *tp;
	hky_conf_t conf;
	hky_pool_t *pool;
	hky_cycle_t *cycle, **old;
	hky_shm_zone_t *shm_zone, *oshm_zone;
	hky_list_part_t *part, *opart;
	hky_open_file_t *file;
	hky_listening_t *ls, *nls;
	hky_core_conf_t *ccf, *old_ccf;
	hky_core_module_t *module;
	char hostname[HKY_MAXHOSTNAMELEN];

	hky_timezone_update();

	tp = hky_timeofday();
	tp->sec = 0;

	hky_time_update();

	log = old_cycle->log;

	pool = hky_create_pool(HKY_CYCLE_POOL_SIZE, log);
	if (NULL == pool) {
		return NULL;
	}
	pool->log = log;

	cycle = hky_pcalloc(pool, sizeof(hky_cycle_t));
	if (NULL == cycle) {
		hky_destroy_pool(pool);
		return NULL;
	}

	cycle->pool = pool;
	cycle->log = log;
	cycle->old_cycle = old_cycle;

	cycle->conf_prefix.len = old_cycle->conf_prefix.len;
	cycle->conf_prefix.data = hky_pstrdup(pool, &old_cycle->conf_prefix);
	if (cycle->conf_prefix.data == NULL) {
		hky_destory_pool(pool);
		return NULL;
	}
	
	cycle->prefix.len = old_cycle->prefix.len;
	cycle->prefix.data = hky_pstrdup(pool, &old_cycle->prefix);
	if (cycle->prefix.data == NULL) {
		hky_destroy_pool(pool);
		return NULL;
	}

	cycle->conf_file.len = old_cycle->conf_file.len;
	cycle->conf_file.data = hky_pnalloc(pool, old_cycle->conf_file.len + 1);
	if (cycle->conf_file.data == NULL) {
		hky_destroy_pool(pool);
		return NULL;
	}
	hky_cpystrn(cycle->conf_file.data, old_cycle->conf_file.data,
		old_cycle->conf_file.len + 1);

	cycle->conf_param.len = old_cycle->conf_param.len;
	cycle->conf_param.data = hky_pstrdup(pool, &old_cycle->conf_param);
	if (cycle->conf_param.data == NULL) {
		hky_destroy_pool(pool);
		return NULL;
	}

	//初始化paths,暂时没用上
	n = old_cycle->paths.nelts ? old_cycle->paths.nelts : 10;
	if (hky_array_init(&cycle->paths, pool, n, sizeof(hky_path_t *)) != HKY_OK) {
		hky_destroy_pool(pool);
		return NULL;
	}
	ngx_memzero(cycle->paths.elts, n * sizeof(hky_path_t *));

	if (hky_array_init(&cycle->config_dump, pool, 1, sizeof(hky_conf_dump_t))
		!= HKY_OK) {
		hky_destroy_pool(pool);
		return NULL;
	}

	hky_rbtree_init(&cycle->config_dump_rbtree, &cycle->config_dump_sentinel,
		hky_str_rbtree_insert_value);

	if (old_cycle->open_files.part.nelts) {
		n = old_cycle->open_files.part.nelts;
		for (part = old_cycle->open_files.part.next; part; part = part->next) {
			n += part->nelts;
		}
	}
	else {
		n = 20;
	}

	if (hky_list_init(&cycle->open_files, pool, n, sizeof(hky_open_file_t)) != HKY_OK) {
		hky_destroy_pool(pool);
		return NULL;
	}

	if (old_cycle->shared_memory.part.nelts) {
		n = old_cycle->shared_memory.part.nelts;
		for (part = old_cycle->shared_memory.part.next; part; part = part->next) {
			n += part->nelts;
		}
	}
	else {
		n = 1;
	}

	if (hky_list_init(&cycle->shared_memory, pool, n, sizeof(hky_shm_zone_t))
		!= HKY_OK) {
		hky_destroy_pool(pool);
		return NULL;
	}

	n = old_cycle->listening.nelts ? old_cycle->listening.nelts : 10;

	if (hky_array_init(&cycle->listening, pool, n, sizeof(hky_listening_t)) != HKY_OK) {
		hky_destroy_pool(pool);
		return NULL;
	}

	hky_memzero(cycle->listening.elts, n * sizeof(hky_listening_t));

	hky_qeueu_init(&cycle->reusable)
}

volatile hky_cycle_t  *hky_cycle;
hky_uint_t             hky_test_config;
hky_uint_t             hky_dump_config;
hky_uint_t             hky_quiet_mode;
