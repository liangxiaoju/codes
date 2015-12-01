#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "klog.h"

#define UNUSE(x) ((void)x)
#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))

struct mylog {
	klog_t *klog;
	kinfo_t **kinfo;
	int num_kinfo;
};

/* XXX add your kinfo here */
extern kinfo_t ssr_kinfo;
extern kinfo_t other_kinfo;
extern kinfo_t mmc_kinfo;

kinfo_t *kinfo[] = {
	&ssr_kinfo,
	&other_kinfo,
	&mmc_kinfo,
};

int limit_file_size(int fd, int size)
{
	struct stat stat;
	int ret = 0;

	if ((fd == STDOUT_FILENO) || (fd == STDERR_FILENO))
		return 0;

	fstat(fd, &stat);
	if (stat.st_size > size) {
		int bytes;
		int cut = stat.st_size/3;
		char *buf = malloc(size);
		if (!buf) {
			perror("malloc");
			return -1;
		}
		lseek(fd, cut, SEEK_SET);
		bytes = read(fd, buf, stat.st_size - cut);
		ftruncate(fd, 0);
		write(fd, buf, bytes);
		free(buf);
		ret = 1;
	}

	return ret;
}

int default_msg_handler(struct klog_msg *msg, void *arg)
{
	char buf[128];
	kinfo_t *kinfo = arg;
	char *logfile = kinfo->file;
	int logsize = kinfo->size;
	int fd = kinfo->fd;

	//fprintf(stdout, "%d %s: %s", msg->id, msg->label, msg->data);

	if (fd <= 0) {
		fd = open(logfile, O_CREAT|O_APPEND|O_RDWR, 0664);
		if (fd < 0)
			fd = STDOUT_FILENO;
		kinfo->fd = fd;
		chmod(logfile, 0664);
	}

	/* kernel time */
	snprintf(buf, sizeof(buf), "%lld.%06lld ",
			msg->ts_nsec/1000/1000000, msg->ts_nsec/1000%1000000);
	if (write(fd, buf, strlen(buf)) < 0)
		perror("write");

	if (write(fd, msg->data, strlen(msg->data)) < 0)
		perror("write");

	limit_file_size(fd, logsize);

	return 0;
}

int main(int argc, char *argv[])
{
	struct mylog *mylog;
	klog_t *klog;
	UNUSE(argc);
	UNUSE(argv);

	fprintf(stdout, "kernel log init.\n");

	mylog = malloc(sizeof(*mylog));
	if (!mylog) {
		fprintf(stderr, "failed to alloc mylog.\n");
		return -1;
	}

	klog = klog_alloc(kinfo, ARRAY_SIZE(kinfo));
	if (!klog) {
		fprintf(stderr, "failed to alloc klog.\n");
		return -1;
	}

	mylog->klog = klog;
	mylog->kinfo = kinfo;
	mylog->num_kinfo = ARRAY_SIZE(kinfo);

	klog_init(klog);

	klog_set_handler(klog, default_custom_msg_handler, klog);

	klog_loop(klog);

	return 0;
}
