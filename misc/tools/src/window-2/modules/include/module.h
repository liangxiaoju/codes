#ifndef __MODULE_H__
#define __MODULE_H__

#include <cdk.h>
#include "win.h"

#define MODULES_DIR "./modules"
#define MODULE_INFO_SYM "module_info_sym"

typedef void *(*handler_t)(void *arg);

typedef struct module {

	int type;

	int level;

	const char *name;

	handler_t handler;

} module_t;

#endif
