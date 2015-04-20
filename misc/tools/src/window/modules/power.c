#include "module.h"
#include "msg.h"
#include <sys/types.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <linux/netlink.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

int uevent_init()
{
	struct sockaddr_nl addr;
	int sz = 64 * 1024;
	int fd;

	memset(&addr,0,sizeof(addr));
	addr.nl_family = AF_NETLINK;
	addr.nl_pid = getpid();
	addr.nl_groups = 0xffffffff;

	fd = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
	if(fd<0) {
		fprintf(stderr, "Failed to create socket !\n");
		return -1;
	}
	setsockopt(fd, SOL_SOCKET, SO_RCVBUFFORCE, &sz, sizeof(sz));

	if(bind(fd, (struct sockaddr *)&addr, sizeof(addr))<0) {
		fprintf(stderr, "Failed to bind socket !\n");
		close(fd);
		return -1;
	}
	return fd;
}

void uevent_monitor(void)
{
	char buf[256];
	int ret = 0, nbytes;
	int fd;

	fd = uevent_init();
	if (fd < 0) {
		exit(-1);
	}

	while ((nbytes = recv(fd, buf, sizeof(buf), 0)) > 0) {
		kill(getppid(), SIGCONT);
	}

	exit(ret);
}

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
	return sysfs_getstr("/sys/class/power_supply/battery/status", status, n);
}

int stop_show_batt = 0;
void sigint_handler(int sig)
{
	stop_show_batt = 1;
}
void sigcout_handler(int sig)
{
}
void show_batt_info(CDKSCREEN *screen)
{
	int i;
	char temp[8][64];
	char *mesg[8];
	CDKLABEL *label;
	pid_t pid;
	int usb_online, ac_online, batt_online;
	int batt_volt, batt_curr, batt_temp, batt_cap;
	char batt_status[16] = {0};

	if ((pid = fork()) == 0) {
		uevent_monitor();
	}

	signal(SIGINT, sigint_handler);
	signal(SIGCONT, sigcout_handler);

	while (!stop_show_batt) {

		usb_online = isUsbOnline();
		ac_online = isAcOnline() && !usb_online;
		batt_online = isBattOnline();
		batt_volt = get_batt_volt();
		batt_curr = get_batt_curr();
		batt_temp = get_batt_temp();
		batt_cap = get_batt_cap();
		get_batt_status(batt_status, sizeof(batt_status));

		sprintf(temp[0], "USB: %s", usb_online ? "online" : "offline");
		sprintf(temp[1], "AC: %s", ac_online ? "online" : "offline");
		sprintf(temp[2], "Batt: %s", batt_online ? "online" : "offline");
		sprintf(temp[3], "Batt Voltage: %dmV", batt_volt);
		sprintf(temp[4], "Batt Current: %dmA", batt_curr);
		sprintf(temp[5], "Batt Temperature: %d C", batt_temp);
		sprintf(temp[6], "Batt Capacity: %d %%", batt_cap);
		sprintf(temp[7], "Batt status: %s", batt_status);

		for (i = 0; i < sizeof(mesg)/sizeof(mesg[0]); i++) {
			mesg[i] = temp[i];
		}

		label = newCDKLabel(screen, CENTER, CENTER, mesg, sizeof(mesg)/sizeof(mesg[0]), TRUE, FALSE);
		drawCDKLabel(label, TRUE);

		sleep(60);

		destroyCDKLabel(label);
	}

	kill(pid, SIGTERM);
	waitpid(pid, 0, 0);

	refreshCDKScreen(screen);
}

int power_handler(WINDOW *win, void *ptr)
{
	CDKSCREEN *screen;
	pid_t pid;
	int key = 0;

	screen = initCDKScreen(win);

	if ((pid = fork()) == 0) {
		show_batt_info(screen);
		exit(0);
	}

	keypad(screen->window, TRUE);
	while ((key = wgetch(screen->window)) != 0) {
		if (key != KEY_UP && key != KEY_DOWN
			&& key != KEY_LEFT && key != KEY_RIGHT) {
			break;
		}
	}

	kill(pid, SIGINT);
	waitpid(pid, 0, 0);

	destroyCDKScreen(screen);

	return 0;
}

module_t module_info_sym = {
	.version = 0,
	.id = "hardware",
	.name = "power",
	.handler = power_handler,
};
