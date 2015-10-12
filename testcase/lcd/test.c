#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <linux/input.h>

#define width	(1024)
#define height	(768)

int show_color(unsigned char red, unsigned char green, unsigned char blue)
{
	int i, j;
	int fd;
	unsigned char buf[height*width*4] = {0};

	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			buf[i*(width*4) + j*4 + 0] = red;
			buf[i*(width*4) + j*4 + 1] = green;
			buf[i*(width*4) + j*4 + 2] = blue;
			buf[i*(width*4) + j*4 + 3] = 0;
		}
	}

	fd = open("/dev/fb0", O_RDWR);
	if (fd < 0) {
		printf("Failed to open fb.\n");
		return -1;
	}

	write(fd, buf, height*width*4);

	close(fd);

	return 0;
}

int get_key(char *dev)
{
	int fd;
	struct input_event event;
	int code;

	fd = open(dev, O_RDONLY);
	if (fd <= 0) {
		printf("Failed to open %s.\n", dev);
		return -1;
	}

	read(fd, &event, sizeof(struct input_event));

	printf("Time:%ld Type:%u Code:%u Value:%d\n",
		event.time.tv_sec * 1000 + event.time.tv_usec,
		event.type, event.code, event.value);

	if (event.value)
		code = event.code;
	else
		code = 0;

	return code;
}

int set_backlight(int val)
{
	FILE *fp;
	char buf[64];

	fp = fopen("/sys/class/backlight/pwm-backlight/brightness", "r+");
	if (fp == NULL) {
		printf("Failed to open backlight.\n");
		return -1;
	}

	snprintf(buf, sizeof(buf), "%d\n", val);

	fputs(buf, fp);

	fclose(fp);

	return 0;
}

int set_system_sleep(int state)
{
	FILE *fp;
	char buf[64];

	fp = fopen("/sys/power/state", "r+");
	if (fp == NULL) {
		printf("Failed to open state.\n");
		return -1;
	}

	snprintf(buf, sizeof(buf), "%s\n", state ? "mem" : "on");

	fputs(buf, fp);

	fclose(fp);

	system("sync");

	return 0;
}

int main(int argc, char *argv[])
{
	int i=0;
	unsigned char red, green, blue;
	unsigned char buf[5][3] = { {255,255,255}, {0,0,0}, {255,0,0}, {0,255,0}, {0,0,255} };

	show_color(255,0,0);

	set_backlight(255);

	if (fork()) {
		while (1) {

			if (get_key("/dev/input/event0")) {
				
				show_color(buf[i][0], buf[i][1], buf[i][2]);
				i++;
				i %= 5;
			}
		}
	} else {
		while (1) {
			if (get_key("/dev/input/event1")) {

				set_system_sleep(1);
				sleep(10);
				set_system_sleep(0);
				show_color(255,255,255);
			}
		}
	}

	return 0;
}

