#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "klog.h"

extern int limit_file_size(int fd, int size);

int ssr_msg_handler(struct klog_msg *msg, void *arg)
{
	char buf[128];
	kinfo_t *kinfo = arg;
	char *logfile = kinfo->file;
	int logsize = kinfo->size;
	int fd = kinfo->fd;
	int i;

	//fprintf(stdout, "%d %s: %s", msg->id, msg->label, msg->data);

	if (fd <= 0) {
		fd = open(logfile, O_CREAT|O_APPEND|O_RDWR|O_SYNC, 0664);
		if (fd < 0)
			fd = STDOUT_FILENO;
		kinfo->fd = fd;
		chmod(logfile, 0664);
	}

	time_t t = time(NULL);
	int offset;
	offset = snprintf(buf, sizeof(buf), "%lld.%06lld",
			msg->ts_nsec/1000/1000000, msg->ts_nsec/1000%1000000);
	strftime(buf+offset, sizeof(buf)-offset, " %Y-%m-%d %H:%M:%S ", localtime(&t));
	if (write(fd, buf, strlen(buf)) < 0)
		perror("write");
	if (write(fd, msg->data, strlen(msg->data)) < 0)
		perror("write");

	limit_file_size(fd, logsize);

	return 0;
}

kinfo_t ssr_kinfo = {
	.group = "ssr",
	.file = "/mobile_info/kernel_record/ssr_reason.txt",
	.size = 256*1024,
	.handler = ssr_msg_handler,
	.fd = -1,
};
