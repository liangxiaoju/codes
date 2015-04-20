#include <unistd.h>
#include <sys/types.h>
#include <sys/mount.h>
#include "module.h"
#include "thread.h"
#include "event.h"

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

int is_mmc_present(void)
{
	return access("/sys/block/mmcblk0/", F_OK) == 0;
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

int get_mmc_mountpath(char *path, int n)
{
	FILE *fp;
	char *line = NULL;
	size_t size;
	int ret = 0;
	char *t;
	int times = 3;

retry:

	fp = fopen("/proc/mounts", "r");
	if (!fp)
		return -1;

	while (getline(&line, &size, fp) > 0) {
		if (strstr(line, "mmc") == NULL)
			continue;
		strtok(line, " ");
		t = strtok(NULL, " ");
		if (!t) {
			ret = 0;
			goto out;
		}
		strncpy(path, t, n);
		ret = 1;
		break;
	}
	fclose(fp);

	if (ret != 1 && times > 0) {
		sleep(1);
		times--;
		goto retry;
	}

out:
	return ret;
}

int sysfs_writes(char *file, char *buf)
{
	FILE *fp;
	int ret = 0;

	fp = fopen(file, "w");
	if (!fp)
		return -1;

	ret = fputs(buf, fp);

	fclose(fp);

	return ret;
}

void handle_info(modwin_t *modwin, WINDOW *win)
{
	char mesg[5][64];
//	char path[64];
	int mmc_online, usb_online;
//	int mounted;
	int i, event;
	int udisk = 0;
	int cols;

	cols = getmaxx(win) - 2;

	while (1) {
		mmc_online = is_mmc_present();
		usb_online = isUsbOnline();
//		mounted = mmc_online && get_mmc_mountpath(path, sizeof(path));

		if (usb_online && mmc_online && udisk) {
//			umount(path);
			sysfs_writes("/sys/devices/platform/dwc_otg/gadget/lun0/file", "/dev/mmcblk0p1");

			snprintf(mesg[0], cols, "CONNECTED");
			snprintf(mesg[1], cols, " ");
			snprintf(mesg[2], cols, " ");
			snprintf(mesg[3], cols, "Hit key[OK]");
			snprintf(mesg[4], cols, "to DISCONNECT to PC.");
		} else {
			snprintf(mesg[0], cols, "SDCard: %s", mmc_online ? "online" : "offline");
			snprintf(mesg[1], cols, " ");
			snprintf(mesg[2], cols, " ");
			if (mmc_online && usb_online) {
				snprintf(mesg[3], cols, "Hit key[OK]");
				snprintf(mesg[4], cols, "to CONNECT to PC.");
			} else {
				snprintf(mesg[3], cols, " ");
				snprintf(mesg[4], cols, " ");
			}
			sysfs_writes("/sys/devices/platform/dwc_otg/gadget/lun0/file", "\n");
			udisk = 0;
		}

		window_lock();
		werase(win);
		box(win, 0, 0);
		for (i = 0; i < sizeof(mesg)/sizeof(mesg[0]); i++) {
			int pos = cols/2 - strlen(mesg[i])/2;
			pos = pos < 0 ? 0 : pos;
			mvwprintw(win, i+3, pos, "%s", mesg[i]);
		}
		wrefresh(win);
		window_unlock();

		event = get_event(modwin);

		if (event == 0x1b)
			exit_subwin(modwin);

		if (event == EVENT_EXIT)
			break;

		if (event == 0xa)
			udisk = !udisk;

	}
}

void *sdcard_handler(void *arg)
{
	modwin_t *modwin = arg;
	WINDOW *win = modwin->win;

	handle_info(modwin, win);

	return 0;
}

module_t module_info_sym = {
	.name = "sdcard",
	.handler = sdcard_handler,
};
