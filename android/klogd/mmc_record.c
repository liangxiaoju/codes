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
extern int default_msg_handler(struct klog_msg *msg, void *arg);

kinfo_t mmc_kinfo = {
	.group = "mmc",
	.file = "/mobile_info/kernel_record/mmc_dump.txt",
	.size = 512*1024,
	.handler = default_msg_handler,
	.fd = -1,
};
