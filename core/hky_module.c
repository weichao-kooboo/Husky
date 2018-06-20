
#include "hky_config.h"
#include "hky_core.h"

hky_uint_t hky_max_module;
static hky_uint_t hky_modules_n;

hky_int_t 
hky_cycle_modules(hky_cycle_t *cycle) {
	cycle->modules = hky_pcalloc(cycle->pool, (hky_max_module + 1)
		* sizeof(hky_module_t *));

	if (cycle->modules == NULL) {
		return HKY_ERROR;
	}

	hky_memcpy(cycle->modules, hky_modules,
		hky_modules_n * sizeof(hky_module_t*));

	cycle->modules_n = hky_modules_n;

	return HKY_OK;
}
hky_int_t 
hky_init_modules(hky_cycle_t *cycle) {
	hky_uint_t i;
	for (i = 0; cycle->modules[i]; i++) {
		if (cycle->modules[i]->init_module) {
			if (cycle->modules[i]->init_module(cycle) != HKY_OK) {
				return HKY_ERROR;
			}
		}
	}
	return HKY_OK;
}