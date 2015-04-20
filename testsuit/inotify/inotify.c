#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>

struct event_str {
	int event;
	const char *str;
} event_str[] = {
	{IN_ACCESS, "File was accessed"},
	{IN_ATTRIB, "Metadata changed"},
	{IN_CLOSE_WRITE, "File opened for writing was closed"},
	{IN_CLOSE_NOWRITE, "File not opened for writing was closed"},
	{IN_CREATE, "File/directory  created  in watched directory"},
	{IN_DELETE, "File/directory deleted from watched directory"},
	{IN_DELETE_SELF, "Watched file/directory was itself deleted"},
	{IN_MODIFY, "File was modified"},
	{IN_MOVE_SELF, "Watched file/directory was itself moved"},
	{IN_MOVED_FROM, "File moved out of watched directory"},
	{IN_MOVED_TO, "File moved into watched directory"},
	{IN_OPEN, "File was opened"},
};

void dump_event(struct inotify_event *event)
{
	int i;

	for (i = 0; i < sizeof(event_str)/sizeof(event_str[0]); i++) {
		if (event_str[i].event & event->mask) {
			printf("EV: '%s' %s\n", event->len ? event->name : "", event_str[i].str);
		}
	}
}

int main(int argc, char *argv[])
{
	struct inotify_event *event;
	char buf[1024];
	int fd, *wd;
	int i, len, ret;
	int event_len;

	fd = inotify_init();
	if (fd == -1) {
		perror("inotify_init");
		return -1;
	}

	wd = malloc((argc -1) * sizeof(int *));
	if (!wd) {
		perror("malloc");
		return -1;
	}

	for (i = 1; i < argc; i++) {
		wd[i - 1] = inotify_add_watch(fd, argv[i], IN_ALL_EVENTS);
	}

	while ((len = read(fd, buf, sizeof(buf))) > 0) {
		event = (struct inotify_event *)buf;
		while (1) {
			dump_event(event);
			event_len = sizeof(struct inotify_event) + event->len;
			if ((len -= event_len) > 0) {
				event = (void *)event + event_len;
			} else {
				break;
			}
		}
	}

	for (i = 1; i < argc; i++) {
		inotify_rm_watch(fd, wd[i - 1]);
	}

	return 0;
}
