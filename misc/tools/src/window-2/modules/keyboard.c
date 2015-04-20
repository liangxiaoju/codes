#include "win.h"
#include "module.h"
#include "thread.h"
#include "event.h"

void *keyboard_handler(void *arg)
{
	modwin_t *modwin = arg;
	WINDOW *win = modwin->win;
	CDKSCREEN *screen;
	CDKSWINDOW *swindow;
//	char *title = "<C>KeyBoard";
	char mesg[128];
	int key;

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	window_lock();
	screen = initCDKScreen(win);
	swindow = newCDKSwindow(screen, CENTER, CENTER, 0, 0, NULL, 100, TRUE, FALSE);
	drawCDKSwindow(swindow, TRUE);
	window_unlock();

	for (;;) {
		key = get_event(modwin);

		if (key == 0x1b)
			exit_subwin(modwin);

		if (key == EVENT_EXIT)
			break;

		if ((key > 0) && (key != EVENT_UEVENT)) {
			snprintf(mesg, sizeof(mesg), "%s", keyname(key));
			window_lock();
			addCDKSwindow(swindow, mesg, BOTTOM);
			drawCDKSwindow(swindow, TRUE);
			window_unlock();
		}

		usleep(1000);
	}

	pthread_exit(NULL);
}

module_t module_info_sym = {
	.name = "keyboard",
	.handler = keyboard_handler,
};
