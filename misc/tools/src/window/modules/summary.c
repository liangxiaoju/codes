#include "module.h"
#include "msg.h"

int summary_handler(WINDOW *win, void *ptr);

module_t module_info_sym = {
	.version = 0,
	.id = "control",
	.name = "summary",
	.handler = summary_handler,
};

int summary_handler(WINDOW *win, void *ptr)
{
	CDKSCREEN *screen;
	CDKSWINDOW *swindow;
	char *title = "<C>Message:";
	module_t *module = &module_info_sym;
	struct msgfunc *msgfunc = module->msgfunc;
	struct message *message;
	char msg[4][256];

	screen = initCDKScreen(win);

	swindow = newCDKSwindow(screen, CENTER, CENTER,
			0, 0, title, 100, TRUE, FALSE);
   	drawCDKSwindow(swindow, ObjOf (swindow)->box);

	for (;;) {
		if (msgfunc->msg_recv(module, &message)) {
			sprintf(msg[0], "From: %s", message->from);
			sprintf(msg[1], "To: %s", message->to);
			sprintf(msg[2], "Text: %s", message->msg);
			sprintf(msg[3], "----------------------");
			addCDKSwindow(swindow, msg[0], BOTTOM);
			addCDKSwindow(swindow, msg[1], BOTTOM);
			addCDKSwindow(swindow, msg[2], BOTTOM);
			addCDKSwindow(swindow, msg[3], BOTTOM);
			msgfunc->freemsg(message);
		} else
			break;
	}
	activateCDKSwindow(swindow, 0);

	destroyCDKSwindow(swindow);
	destroyCDKScreen(screen);

	return 0;
}

