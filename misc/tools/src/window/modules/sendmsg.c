#include "module.h"
#include "msg.h"

int sendmsg_handler(WINDOW *win, void *ptr);

module_t module_info_sym = {
	.version = 0,
	.id = "control",
	.name = "sendmsg",
	.handler = sendmsg_handler,
};

int sendmsg_handler(WINDOW *win, void *ptr)
{
	CDKSCREEN *screen;
	CDKENTRY *msg_entry, *to_entry;
	char *title[] = {"<C>Enter your message", "<C>Enter addressee"};
	char *label[] = {"</U/5>TEXT: <!U!5>", "</U/5>To: <!U!5>"};
	char *text, *addressee;
	module_t *module = &module_info_sym;
	struct msgfunc *msgfunc = module->msgfunc;
	struct message *message;
	int lines, cols;

	screen = initCDKScreen(win);
	getmaxyx(win, lines, cols);

	msg_entry = newCDKEntry(screen, CENTER, CENTER,
			title[0], label[0], A_NORMAL, '.',
			vMIXED, cols-10, 0, 256, TRUE, FALSE);
	to_entry = newCDKEntry(screen, CENTER, CENTER,
			title[1], label[1], A_NORMAL, '.',
			vMIXED, cols-10, 0, 256, TRUE, FALSE);

	for (;;) {
		cleanCDKEntry(msg_entry);
		text = activateCDKEntry(msg_entry, 0);
		eraseCDKEntry(msg_entry);
		if (msg_entry->exitType == vNORMAL) {
			cleanCDKEntry(to_entry);
			addressee = activateCDKEntry(to_entry, 0);
			eraseCDKEntry(to_entry);
			if (to_entry->exitType == vNORMAL) {
				char *info[5];
				char tmp[5][256];
				message = msgfunc->mkmsg(module->name, addressee, text, NULL);
				sprintf(tmp[0], "From: %s", message->from);
				sprintf(tmp[1], "To: %s", message->to);
				sprintf(tmp[2], "Text: %s", message->msg);
				sprintf(tmp[3], "Sending message ...");
				sprintf(tmp[4], "Hit any key to continue.");
				info[0] = tmp[0];
				info[1] = tmp[1];
				info[2] = tmp[2];
				info[3] = tmp[3];
				info[4] = tmp[4];
				popupLabel(screen, (CDK_CSTRING2)info, 5);
				msgfunc->msg_send(module, message);
			}
		} else {
			break;
		}
	}

	destroyCDKEntry(msg_entry);
	destroyCDKEntry(to_entry);
	destroyCDKScreen(screen);

	return 0;
}

