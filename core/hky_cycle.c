
#include "hky_config.h"
#include "hky_core.h"

static void hky_destroy_cycle_pools(hky_conf_t *conf);
static hky_int_t hky_test_lockfile(hky_uchar *file, hky_log_t *log);
static hky_int_t hky_init_zone_pool(hky_cycle_t *cycle, hky_shm_zone_t *shm_zone);

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

	hky_qeueu_init(&cycle->reusable_connections_queue);

	cycle->conf_ctx = hky_pcalloc(pool, hky_max_module * sizeof(void*));
	if (cycle->conf_ctx == NULL) {
		hky_destory_pool(pool);
		return NULL;
	}

	if (gethostname(hostname, HKY_MAXHOSTNAMELEN) == -1) {
		hky_log_error(HKY_LOG_EMERG, log, hky_errno, "gethostname() failed");
		hky_destroy_pool(pool);
		return NULL;
	}

	hostname[HKY_MAXHOSTNAMELEN - 1] = '\0';
	cycle->hostname.len = hky_strlen(hostname);

	cycle->hostname.data = hky_pnalloc(pool, cycle->hostname.len);
	if (cycle->hostname.data == NULL) {
		hky_destroy_pool(pool);
		return NULL;
	}

	hky_strlow(cycle->hostname.data, (hky_uchar*)hostname, cycle->hostname.len);

	if (hky_cycle_module(cycle) != HKY_OK) {
		hky_destroy_pool(pool);
		return NULL;
	}

	for (i = 0; cycle->modules[i]; i++) {
		if (cycle->modules[i]->type != HKY_CORE_MODULE) {
			continue;
		}
		module = cycle->modules[i]->ctx;

		if (module->create_conf) {
			rv = module->create_conf(cycle);
			if (rv == NULL) {
				hky_destroy_pool(pool);
				return NULL;
			}
			cycle->conf_ctx[cycle->modules[i]->index] = rv;
		}
	}

	senv = environ;

	hky_memzero(&conf, sizeof(hky_conf_t));

	conf.args = hky_array_create(pool, 10, sizeof(hky_str_t));
	if (conf.args == NULL) {
		hky_destory_pool(pool);
		return NULL;
	}

	conf.temp_pool = hky_create_pool(HKY_CYCLE_POOL_SIZE, log);
	if (conf.temp_pool == NULL) {
		hky_destory_pool(pool);
		return NULL;
	}

	conf.ctx = cycle->conf_ctx;
	conf.cycle = cycle;
	conf.pool = pool;
	conf.log = log;
	conf.module_type = HKY_CORE_MODULE;
	conf.cmd_type = HKY_MAIN_CONF;

#if 0
	log->log_level = HKY_LOG_DEBUG_ALL;
#endif // 0

	if (hky_conf_param(&conf) != HKY_CONF_OK) {
		environ = senv;
		hky_destroy_cycle_pools(&conf);
		return NULL;
	}
	if (hky_conf_parse(&conf, &cycle->conf_file) != HKY_CONF_OK) {
		environ = senv;
		hky_destroy_cycle_pools(&conf);
		return NULL;
	}
	if (hky_test_config && !hky_quiet_mode) {
		hky_log_stderr(0, "the configuration file %s syntax is ok",
			cycle->conf_file.data);
	}
	for (i = 0; cycle->modules[i]; i++)
	{
		if (cycle->modules[i]->type != HKY_CORE_MODULE) {
			continue;
		}
		module = cycle->modules[i]->ctx;
		if (module->init_conf) {
			if (module->init_conf(cycle, cycle->conf_ctx[cycle->modules[i]->index]) == HKY_CONF_ERROR) {
				environ = senv;
				hky_destroy_cycle_pools(&conf);
				return NULL;
			}
		}
	}
	if (hky_process == HKY_PROCESS_SIGNALLER) {
		return cycle;
	}

	ccf = (hky_core_conf_t *)hky_get_conf(cycle->conf_ctx, hky_core_module);

	if (hky_test_config) {
		if (hky_create_pidfile(&ccf->pid, log) != HKY_OK) {
			goto failed;
		}
	}
	else if (!hky_is_init_cycle(old_cycle)) {
		old_ccf = (hky_core_conf_t *)hky_get_conf(old_cycle->conf_ctx, hky_core_module);

		if (ccf->pid.len != old_ccf->pid.len
			|| hky_strcmp(ccf->pid.data, old_ccf->pid.data) != 0) {
			if (hky_create_pidfile(&ccf->pid, log) != HKY_OK) {
				goto failed;
			}
			hky_delete_pidfile(old_cycle);
		}
	}
	
	if (hky_test_lockfile(cycle->lock_file.data, log) != HKY_OK) {
		goto failed;
	}

	if (hky_create_paths(cycle, ccf->user) != HKY_OK) {
		goto failed;
	}

	if (hky_log_open_default(cycle) != HKY_OK) {
		goto failed;
	}

	part = &cycle->open_files.part;
	file = part->elts;

	for (i = 0; ; i++) {
		if (i >= part->nelts) {
			if (part->next == NULL) {
				break;
			}
			part = part->next;
			file = part->elts;
			i = 0;
		}
		if (file[i].name.len == 0) {
			continue;
		}
		file[i].fd = hky_open_file(file[i].name.data,
			HKY_FILE_APPEND,
			HKY_FILE_CREATE_OR_OPEN,
			HKY_FILE_DEFAULT_ACCESS);
		hky_log_debug3(HKY_LOG_DEBUG_CORE, log, 0,
			"log: %p %d \"%s\"",
			&file[i], file[i].fd, file[i].name.data);

		if (file[i].fd == HKY_INVALID_FILE) {
			hky_log_error(HKY_LOG_EMERG, log, hky_errno,
				hky_open_file_n " \"%s\" failed",
				file[i].name.data);
			goto failed;
		}
#if !(HKY_WIN32)
		if (fcntl(file[i].fd, F_SETFD, FD_CLOEXEC) == -1) {
			hky_log_error(HKY_LOG_EMERG, log, hky_errno,
				"fcntl(FD_CLOEXEC) \"%s\" failed",
				file[i].name.data);
			goto failed;
		}
#endif // !(HKY_WIN32)
	}
	cycle->log = &cycle->new_log;
	pool->log = &cycle->new_log;

	part = &cycle->shared_memory.part;
	shm_zone = part->elts;

	for (i = 0;; i++) {
		if (i >= part->nelts) {
			if (part->next == NULL) {
				break;
			}
			part = part->next;
			shm_zone = part->elts;
			i = 0;
		}

		if (shm_zone[i].shm.size == 0) {
			hky_log_error(HKY_LOG_EMERG, log, 0,
				"zero size shared memory zone \"%V\"",
				&shm_zone[i].shm.name);
			goto failed;
		}
		shm_zone[i].shm.log = cycle->log;

		opart = &old_cycle->shared_memory.part;
		oshm_zone = opart->elts;

		for (n = 0;; n++)
		{
			if (n >= opart->nelts) {
				if (opart->next == NULL) {
					break;
				}
				opart = opart->next;
				oshm_zone = opart->elts;
				n = 0;
			}

			if (shm_zone[i].shm.name.len != oshm_zone[n].shm.name.len) {
				continue;
			}

			if (hky_strncmp(shm_zone[i].shm.name.data,
				oshm_zone[n].shm.name.data,
				shm_zone[i].shm.name.len)
				!= 0) {
				continue;
			}

			if (shm_zone[i].tag == oshm_zone[n].tag
				&&shm_zone[i].shm.size == oshm_zone[n].shm.size
				&& !shm_zone[i].noreuse) {
				shm_zone[i].shm.addr = oshm_zone[n].shm.addr;
#if (HKY_WIN32)
				shm_zone[i].shm.handle = oshm_zone[n].shm.handle;
#endif // (HKY_WIN32)

				if (shm_zone[i].init(&shm_zone[i], oshm_zone[n].data) != HKY_OK) {
					goto failed;
				}
				goto shm_zone_found;
			}
			break;
		}
		if (hky_shm_alloc(&shm_zone[i].shm) != HKY_OK) {
			goto failed;
		}
		if(hky_init_zone_)
	}
    
    return NULL;
}

static void 
hky_destroy_cycle_pools(hky_conf_t *conf) {
	hky_destory_pool(conf->temp_pool);
	hky_destory_pool(conf->pool);
}

static hky_int_t 
hky_init_zone_pool(hky_cycle_t *cycle, hky_shm_zone_t *zn) {
	hky_uchar *file;
	hky_slab_pool_t *sp;
	sp = (hky_slab_pool_t *)zn->shm.addr;

	if (zn->shm.exists) {
		if (sp == sp->addr) {
			return HKY_OK;
		}
#if (HKY_WIN32)
		if (hky_shm_remap(&zn->shm, sp->addr) != HKY_OK) {
			return HKY_ERROR;
		}
		sp = (hky_slab_pool_t *)zn->shm.addr;
		if (sp == sp->addr) {
			return HKY_OK;
		}
#endif // (HKY_WIN32)
		hky_log_error(HKY_LOG_EMERG, cycle->log, 0,
			"shared zone \"%V\" has no equal addresses:%p vs %p",
			&zn->shm.name, sp->addr, sp);
		return HKY_ERROR;
	}
	sp->end = zn->shm.addr + zn->shm.size;
	sp->min_shift = 3;
	sp->addr = zn->shm.addr;

#if (HKY_HAVE_ATOMIC_OPS)
	file = NULL;
#else
	file = hky_pnalloc(cycle->pool, cycle->lock_file.len + zn->shm.name.len);
	if (file == NULL) {
		return HKY_ERROR;
	}
	(void)hky_sprintf(file, "%V%V%Z", &cycle->lock_file, &zn->shm.name);
#endif // (HKY_HAVE_ATOMIC_OPS)

	if (hky_shmtx_create(&sp->mutex, &sp->lock, file) != HKY_OK) {
		return HKY_ERROR;
	}
	hky_slab_init(sp);

	return HKY_OK;
}

hky_int_t 
hky_create_pidfile(hky_str_t *name, hky_log_t *log) {
	size_t len;
	hky_uint_t create;
	hky_file_t file;
	hky_uchar	pid[HKY_INT64_LEN + 2];

	if (hky_process > HKY_PROCESS_MASTER) {
		return HKY_OK;
	}
	hky_memzero(&file, sizeof(hky_file_t));
	file.name = *name;
	file.log = log;

	create = hky_test_config ? HKY_FILE_CREATE_OR_OPEN : HKY_FILE_TRUNCATE;
	file.fd = hky_open_file(file.name.data, HKY_FILE_RDWR, create, HKY_FILE_DEFAULT_ACCESS);
	if (file.fd == HKY_INVALID_FILE) {
		hky_log_error(HKY_LOG_EMERG, log, hky_errno,
			hky_open_file_n "\"%s\" failed", file.name.data);
		return HKY_ERROR;
	}
	if (!hky_test_config) {
		len = hky_snprintf(pid, HKY_INT64_LEN + 2, "%P%N", hky_pid) - pid;
		if (hky_write_file(&file, pid, len, 0) == HKY_ERROR) {
			return HKY_ERROR;
		}
	}
	if (hky_close_file(file.fd) == HKY_FILE_ERROR) {
		hky_log_error(HKY_LOG_ALERT, log, hky_errno,
			hky_close_file_n " \"%s\" failed", file.name.data);
	}
	return HKY_OK;
}

void 
hky_delete_pidfile(hky_cycle_t *cycle) {
	hky_uchar	*name;
	hky_core_conf_t *ccf;
	ccf = (hky_core_conf_t*)hky_get_conf(cycle->conf_ctx, hky_core_module);
	name = hky_new_binary ? ccf->oldpid.data : ccf->pid.data;
	if (hky_delete_file(name) == HKY_FILE_ERROR) {
		hky_log_error(HKY_LOG_ALERT, cycle->log, hky_errno,
			hky_delete_file_n "\"%s\" failed", name);
	}
}


static 
hky_int_t hky_test_lockfile(hky_uchar *file, hky_log_t *log) {
#if !(HKY_HAVE_ATOMIC_OPS)
	hky_fd_t	fd;
	fd = hky_open_file(file, HKY_FILE_RDWR, HKY_FILE_CREATE_OR_OPEN,
		HKY_FILE_DEFAULT_ACCESS);
	if (fd == HKY_INVALID_FILE) {
		hky_log_error(HKY_LOG_EMERG, log, hky_errno,
			hky_open_file_n "\"%s\" failed", file);
		return HKY_ERROR;
	}
	if (hky_close_file(fd) == HKY_FILE_ERROR) {
		hky_log_error(HKY_LOG_ALERT, log, hky_errno,
			hky_close_file_n "\"%s\" failed", file);
	}
	if (hky_delete_file(file) == HKY_FILE_ERROR) {
		hky_log_error(HKY_LOG_ALERT, log, hky_errno,
			hky_delete_file_n "\"%s\" failed", file);
	}
#endif // !(HKY_HAVE_ATOMIC_OPS)
	return HKY_OK;
}

volatile hky_cycle_t  *hky_cycle;
hky_uint_t             hky_test_config;
hky_uint_t             hky_dump_config;
hky_uint_t             hky_quiet_mode;
