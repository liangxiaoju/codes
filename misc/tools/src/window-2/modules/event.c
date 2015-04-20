#include "module.h"
#include "event.h"
#include "thread.h"
#include <stdio.h>
#include <stdlib.h>

modwin_t *modwin;

int sysfs_getstr(char *file, char *buf, int n)
{
	FILE *fp;
	int ret = 0;

	fp = fopen(file, "r");
	if (!fp)
		return -1;

	fgets(buf, n, fp);

	fclose(fp);

	return ret;
}

int isHeadPhoneOnline(void)
{
	char buf[2] = {0};
	sysfs_getstr("/sys/class/switch/h2w/state", buf, sizeof(buf));
	if (!strncmp(buf, "1", 1)) 
		return 1;
	else
		return 0;
}

int isUsbOnline(void)
{
	char buf[2] = {0};
	sysfs_getstr("/sys/class/power_supply/usb/online", buf, sizeof(buf));
	if (!strncmp(buf, "1", 1)) 
		return 1;
	else
		return 0;
}

int isAcOnline(void)
{
	char buf[2] = {0};
	sysfs_getstr("/sys/class/power_supply/ac/online", buf, sizeof(buf));
	if (!strncmp(buf, "1", 1)) 
		return 1;
	else
		return 0;
}

int isBattOnline(void)
{
	char buf[2] = {0};
	sysfs_getstr("/sys/class/power_supply/battery/present", buf, sizeof(buf));
	if (!strncmp(buf, "1", 1)) 
		return 1;
	else
		return 0;
}

int get_batt_volt(void)
{
	char buf[8] = {0};
	int volt;
	sysfs_getstr("/sys/class/power_supply/battery/batt_vol", buf, sizeof(buf));
	volt = strtol(buf, 0, 0);
	return volt;
}

int get_batt_curr(void)
{
	char buf[8] = {0};
	int curr;
	sysfs_getstr("/sys/class/power_supply/battery/current_now", buf, sizeof(buf));
	curr = strtoul(buf, 0, 10);
	return curr;
}

int get_batt_temp(void)
{
	char buf[8] = {0};
	int temp;
	sysfs_getstr("/sys/class/power_supply/battery/batt_temp", buf, sizeof(buf));
	temp = strtoul(buf, 0, 10);
	return temp;
}

int get_batt_cap(void)
{
	char buf[8] = {0};
	int cap;
	sysfs_getstr("/sys/class/power_supply/battery/capacity", buf, sizeof(buf));
	cap = strtoul(buf, 0, 10);
	return cap;
}

int get_batt_status(char *status, int n)
{
	sysfs_getstr("/sys/class/power_supply/battery/status", status, n);
	status[strlen(status)-1] = '\0';
	return 0;
}

void show_event_info(WINDOW *win)
{
	int i;
	char mesg[8][64];
	int usb_online, ac_online, batt_online;
	int batt_volt, batt_curr, batt_temp, batt_cap, headphone;
	char batt_status[16];
	int event;
	int cols;

	cols = getmaxx(win) - 2;

	while (1) {

		headphone = isHeadPhoneOnline();
		usb_online = isUsbOnline();
		ac_online = isAcOnline() && !usb_online;
		batt_online = isBattOnline();
		batt_volt = get_batt_volt();
		batt_curr = get_batt_curr();
		batt_temp = get_batt_temp();
		batt_cap = get_batt_cap();
		get_batt_status(batt_status, sizeof(batt_status));

		snprintf(mesg[0], cols, "HeadPhone: %s", headphone ? "online" : "offline");
		snprintf(mesg[1], cols, "USB: %s", usb_online ? "online" : "offline");
		snprintf(mesg[2], cols, "AC: %s", ac_online ? "online" : "offline");
		snprintf(mesg[3], cols, "Batt:");
		snprintf(mesg[4], cols, "Volt: %dmV", batt_volt);
		snprintf(mesg[5], cols, "Capacity: %d %%", batt_cap);
		snprintf(mesg[6], cols, "Status: %s", batt_status);
		snprintf(mesg[7], cols, " ");

		window_lock();
		werase(win);
		box(win, 0, 0);
		for (i = 0; i < sizeof(mesg)/sizeof(mesg[0]); i++) {
			mvwprintw(win, i+2, 1, "%s ", mesg[i]);
		}
		wrefresh(win);
		window_unlock();

		event = get_event(modwin);

		if (event == 0x1b)
			exit_subwin(modwin);

		if (event == EVENT_EXIT)
			break;
	}
}

void *event_handler(void *arg)
{
	modwin = arg;
	WINDOW *win = modwin->win;

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	show_event_info(win);

	pthread_exit(NULL);
}

module_t module_info_sym = {
	.name = "event",
	.handler = event_handler,
};
