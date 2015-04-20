#include <dirent.h>
#include <dlfcn.h>
#include "win.h"
#include "msg.h"

int insert_tag(mainwin_t *mainwin, module_t *module)
{
	int i;
	barwin_t *barwin = &mainwin->bar;

	for (i = 0; i < barwin->nbutton; i++) {
		if (strcmp(barwin->buttonList[i], module->id) == 0) {
			return -1;
		}
	}
	barwin->buttonList[barwin->nbutton] = copyChar(module->id);
	barwin->nbutton++;
	mainwin->ntag = barwin->nbutton;
	return 0;
}

int insert_item(selectwin_t *selectwin, module_t *module)
{
	selectwin->itemList[selectwin->nchoise] = copyChar(module->name);
	selectwin->choise[selectwin->nchoise].name = selectwin->itemList[selectwin->nchoise];
	selectwin->choise[selectwin->nchoise].handler = module->handler;
	selectwin->nchoise++;
	return 0;
}

struct msgfunc msgfunc = {
	.mkmsg		= mkmsg,
	.freemsg	= freemsg,
	.msg_send	= msg_send,
	.msg_recv	= msg_recv,
};

int set_module_info(mainwin_t *mainwin, module_t **modules, int n)
{
	int i;

	msg_init(&mainwin->msg_list);
	for (i = 0; i < n; i++) {
		modules[i]->msg_list = &mainwin->msg_list;
		modules[i]->msgfunc = &msgfunc;
	}

	return 0;
}

int save_module_info(mainwin_t *mainwin, module_t **modules, int n)
{
	int i, j;
	barwin_t *barwin = &mainwin->bar;

	for (i = 0; i < n; i++) {
		insert_tag(mainwin, modules[i]);
	}

	for (i = 0; i < n; i++) {
		for (j = 0; j < mainwin->ntag; j++) {
			if (strcmp(barwin->buttonList[j], modules[i]->id) == 0) {
				insert_item(&mainwin->tag[j].selectwin, modules[i]);
			}
		}
	}

	set_module_info(mainwin, modules, n);

	mainwin->nmodules = n;

	return 0;
}

int modules_filter(const struct dirent *dirent)
{
	return strstr(dirent->d_name, ".so") ? 1 : 0;
}

int get_modules(mainwin_t *mainwin)
{
	char libpath[128];
	void *handle;
	struct dirent **dirent;
	int n, i, k;
	module_t **modules = mainwin->modules;

	n = scandir(MODULES_DIR, &dirent, modules_filter, alphasort);
	if (n > MAX_MODULE)
		n = MAX_MODULE;
	if (n < 0)
		perror("scandir");
	else {
		for (i = k = 0; i < n; i++) {
			snprintf(libpath, sizeof(libpath), "%s/%s", MODULES_DIR, dirent[i]->d_name);
			free(dirent[i]);
			handle = dlopen(libpath, RTLD_NOW);
			if (!handle)
				continue;
			modules[k] = (module_t *)dlsym(handle, MODULE_INFO_SYM);
			if (!modules[k]) {
				dlclose(handle);
				continue;
			}
			k++;
		}
	}
	free(dirent);

	save_module_info(mainwin, modules, k);

	return k;
}
