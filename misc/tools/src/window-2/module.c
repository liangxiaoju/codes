#include <dirent.h>
#include <dlfcn.h>
#include "win.h"

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
	module_t **module;

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
			if (!handle) {
				fprintf(stderr, "%s\n", dlerror());
				continue;
			}
			module = &mainwin->box.subwin[k].module;
			*module = (module_t *)dlsym(handle, MODULE_INFO_SYM);
			if (!(*module)) {
				fprintf(stderr, "%s\n", dlerror());
				dlclose(handle);
				continue;
			}
			k++;
		}
	}
	free(dirent);

	mainwin->box.nsubs = k;

	return k;
}
