#include "module.h"
#include "msg.h"

module_t module_info_sym;

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
	char *title = "<C>Wifi Info";
	char *mesg;
	CDKSWINDOW *swindow;

	swindow = newCDKSwindow(screen, CENTER, CENTER,
			0, 0, title, 100, TRUE, FALSE);

	mesg = "<C>Use the Arrow to move window.";
	addCDKSwindow(swindow, mesg, BOTTOM);
	mesg = "<C>Hit Enter to exit.";
	addCDKSwindow(swindow, mesg, BOTTOM);

	execCDKSwindow(swindow, "iwlist scanning 2>/dev/null", BOTTOM);

	activateCDKSwindow(swindow, 0);

	destroyCDKSwindow(swindow);
	return 0;
}

int wifi_handler(WINDOW *win, void *ptr)
{
	CDKSCREEN *screen;

	screen = initCDKScreen(win);

	load_driver();

	show_info(screen);

	unload_driver();

	destroyCDKScreen(screen);

	return 0;
}

module_t module_info_sym = {
	.version = 0,
	.id = "hardware",
	.name = "wifi",
	.handler = wifi_handler,
};
