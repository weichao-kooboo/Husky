#ifndef HKY_MODULE_H_INCLUDED
#define HKY_MODULE_H_INCLUDED

#include "hky_config.h"
#include "hky_core.h"
#include "husky.h"

struct hky_module_s{
	hky_uint_t	ctx_index;
	//标记模块的索引
	hky_uint_t index;
	//模块上下文
	void *ctx;
	hky_command_t	*commands;
	//模块类型,用于过滤模块
	hky_uint_t type;
};
typedef struct{
    hky_str_t name;
    void *(*create_conf)(hky_cycle_t *cycle);
    char *(*init_conf)(hky_cycle_t *cycle,void *conf);
} hky_core_module_t;

hky_int_t hky_cycle_modules(hky_cycle_t *cycle);

extern hky_module_t *hky_modules[];
extern hky_uint_t hky_max_module;

#endif // HKY_MODULE_H_INCLUDED
