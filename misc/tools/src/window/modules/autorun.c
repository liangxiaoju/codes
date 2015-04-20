#include "win.h"
#include "module.h"
#include "msg.h"

module_t module_info_sym;

int send_msg(const char *to, char *msg)
{
	module_t *module = &module_info_sym;
	struct msgfunc *msgfunc = module->msgfunc;
	struct message *message;

	message = msgfunc->mkmsg(module->name, to, msg, NULL);
	msgfunc->msg_send(module, message);
	return 0;
}

int run_modules(CDKSCREEN *screen, mainwin_t *mainwin)
{
	int i;
	char *mesg[2];
	char temp[32];

	for (i = 0; i < mainwin->nmodules; i++) {
		module_t *module = mainwin->modules[i];

		if (strcmp(module->id, "hardware") == 0) {
			snprintf(temp, 32, "<C>Running %s Test ...", module->name);
			mesg[0] = temp;
			mesg[1] = "<C>Hit any key to continue.";
			popupLabel(screen, mesg, 2);

			send_msg(module->name, "AUTORUN");
			module->handler(screen->window, mainwin);
		}
	}

	mesg[0] = "<C>Auto Test Completed.";
	mesg[1] = "<C>Hit any key to continue.";
	popupLabel(screen, mesg, 2);

	return 0;
}

int autorun_handler(WINDOW *win, void *ptr)
{
	char *mesg[2];
	CDKSCREEN *screen;

	screen = initCDKScreen(win);

	mesg[0] = "<C>Auto Running Hardware Test ...";
	mesg[1] = "<C>Hit any key to continue.";
	popupLabel(screen, mesg, 2);

	run_modules(screen, ptr);

	destroyCDKScreen(screen);

	return 0;
}

module_t module_info_sym = {
	.version = 0,
	.id = "control",
	.name = "autorun",
	.handler = autorun_handler,
};
