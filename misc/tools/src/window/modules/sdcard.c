#include <unistd.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include "module.h"
#include "msg.h"

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

void handle_uevent(void)
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

	while (1);
	exit(ret);
}

int mmc_state_change = 1;
void sigcout_handler(int sig)
{
	mmc_state_change = 1;
}

int should_return = 0;
void sigint_handler(int sig)
{
	should_return = 1;
}

int is_mmc_present(void)
{
	return access("/sys/block/mmcblk0/", F_OK) == 0;
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

	if (should_return != 1 && ret != 1 && times > 0) {
		sleep(1);
		times--;
		goto retry;
	}

out:
	return ret;
}

int check_mmc_read(void)
{
	return 1;
}

int check_mmc_write(void)
{
	return 1;
}

void handle_info(CDKSCREEN *screen)
{
	pid_t pid;
	CDKLABEL *label;
	char path[64];
	char *mesg[5];
	int i;

	if ((pid = fork()) == 0) {
		handle_uevent();
	}

	signal(SIGINT, sigint_handler);
	signal(SIGCONT, sigcout_handler);

	for (i = 0; i < sizeof(mesg)/sizeof(mesg[0]); i++)
		mesg[i] = "                         ";
	label = newCDKLabel(screen, CENTER, CENTER, mesg, 5, TRUE, FALSE);

	for (;;) {
		int online, mounted, read, write;
		char temp[5][64];

		if (mmc_state_change) {

			mmc_state_change = 0;

			online = mounted = read = write = 0;
			memset(path, 0, sizeof(path));

			online = is_mmc_present();
			mounted = online && get_mmc_mountpath(path, sizeof(path));
			read = mounted && check_mmc_read();
			write = mounted && check_mmc_write();

			sprintf(temp[0], "<C>Info");
			sprintf(temp[1], "status: %s", online ? "online" : "offline");
			sprintf(temp[2], "mounted: %s", mounted ? "TRUE" : "FALSE");
			sprintf(temp[3], "read: %s", read ? "OK" : "ERROR");
			sprintf(temp[4], "write: %s", write ? "OK" : "ERROR");
			mesg[0] = temp[0];
			mesg[1] = temp[1];
			mesg[2] = temp[2];
			mesg[3] = temp[3];
			mesg[4] = temp[4];

			eraseCDKLabel(label);
			setCDKLabelMessage(label, mesg, 5);
			drawCDKLabel(label, TRUE);
		}

		if (should_return)
			break;

		sleep(60);
	}

	kill(pid, SIGTERM);
	waitpid(pid, 0, 0);

	destroyCDKLabel(label);
	refreshCDKScreen(screen);
}

int sdcard_handler(WINDOW *win, void *ptr)
{
	CDKSCREEN *screen;
	pid_t pid;
	int key;

	screen = initCDKScreen(win);

	if ((pid = fork()) == 0) {
		handle_info(screen);
		exit(0);
	}

	keypad(screen->window, TRUE);
	while ((key = wgetch(screen->window)) != ERR) {
		if (key) {
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
	.name = "sdcard",
	.handler = sdcard_handler,
};
