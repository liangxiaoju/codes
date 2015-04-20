#include "thread.h"
#include "module.h"
#include "event.h"

module_t module_info_sym;

modwin_t *modwin;

int insmod(const char *filename, const char *args)
{
	return 0;
}

int rmmod(const char *modname)
{
	return 0;
}

int poweron(void)
{
#if __arm__
	system("echo ON > /sys/devices/platform/wifi/wifi_power");
#endif
	return 0;
}

int poweroff(void)
{
#if __arm__
	system("echo OFF > /sys/devices/platform/wifi/wifi_power");
#endif
	return 0;
}

int load_driver()
{
	poweron();
	insmod(0, 0);
	sleep(1);
	return 0;
}

int unload_driver()
{
	rmmod(0);
	poweroff();
	return 0;
}

int show_info(CDKSCREEN *screen)
{
//	char *title = "<C>Wifi Info";
	char *mesg;
	CDKSWINDOW *swindow;

	window_lock();
	swindow = newCDKSwindow(screen, CENTER, CENTER,
			0, 0, NULL, 100, TRUE, FALSE);

	mesg = "<C>Use the Arrow to move window.";
	addCDKSwindow(swindow, mesg, BOTTOM);
	mesg = "<C>Hit Enter to exit.";
	addCDKSwindow(swindow, mesg, BOTTOM);
	drawCDKSwindow(swindow, TRUE);
	window_unlock();

	window_lock();
	execCDKSwindow(swindow, "iwlist scanning 2>&1", BOTTOM);
	window_unlock();

	while (1) {
		int key;
		key = get_event(modwin);
		if (!key)
			continue;
		if (key == EVENT_EXIT)
			break;
		window_lock();
		injectCDKSwindow(swindow, key);
		window_unlock();
		if (key == 0x1b)
			exit_subwin(modwin);
	}

	window_lock();
	destroyCDKSwindow(swindow);
	window_unlock();
	return 0;
}

void *wifi_handler(void *arg)
{
	modwin = arg;
	WINDOW *win = modwin->win;
	CDKSCREEN *screen;

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	window_lock();
	screen = initCDKScreen(win);
	window_unlock();

	load_driver();

	show_info(screen);

	unload_driver();

	window_lock();
	destroyCDKScreen(screen);
	window_unlock();

	pthread_exit(NULL);
}

module_t module_info_sym = {
	.name = "wifi",
	.handler = wifi_handler,
};
