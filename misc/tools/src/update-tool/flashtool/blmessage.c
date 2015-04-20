#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#define BYTE2SEC(x) ( (((x)-1)>>9) + 1 )
#define MTD_PROC_FILENAME   "/proc/mtd"
#define MISC_PAGES 3
#define MISC_COMMAND_PAGE 1
#define RKFLASH_BLOCKSIZE (32*512)

struct bootloader_message {
    char command[32];
    char status[32];
    char recovery[1024];
};

/* for rk2918 */
int rkflash_read_by_block(int fd, void *buf, int nbytes)
{
	int i, ret;
	int blocks = BYTE2SEC(nbytes) / BYTE2SEC(RKFLASH_BLOCKSIZE);

	for (i = 0; i < blocks; i++) {
		ret = read(fd, buf, BYTE2SEC(RKFLASH_BLOCKSIZE));
		if (ret != BYTE2SEC(RKFLASH_BLOCKSIZE))
			return -1;
		buf += RKFLASH_BLOCKSIZE;
	}
	return nbytes;
}

int rkflash_write_by_block(int fd, void *buf, int nbytes)
{
	int i, ret;
	int blocks = nbytes / RKFLASH_BLOCKSIZE;

	for (i = 0; i < blocks; i++) {
		ret = write(fd, buf, BYTE2SEC(RKFLASH_BLOCKSIZE));
		if (ret != BYTE2SEC(RKFLASH_BLOCKSIZE))
			return -1;
		buf += RKFLASH_BLOCKSIZE;
	}
	return nbytes;
}

/* */
int rkflash_read(int fd, void *buf, int nbytes)
{
	int i, ret;
	int blocks = BYTE2SEC(nbytes) / BYTE2SEC(RKFLASH_BLOCKSIZE);

	for (i = 0; i < blocks; i++) {
		ret = read(fd, buf, RKFLASH_BLOCKSIZE);
		if (ret != RKFLASH_BLOCKSIZE)
			return -1;
		buf += RKFLASH_BLOCKSIZE;
	}
	return nbytes;
}

int rkflash_write(int fd, void *buf, int nbytes)
{
	int i, ret;
	int blocks = nbytes / RKFLASH_BLOCKSIZE;

	for (i = 0; i < blocks; i++) {
		ret = write(fd, buf, RKFLASH_BLOCKSIZE);
		if (ret != RKFLASH_BLOCKSIZE)
			return -1;
		buf += RKFLASH_BLOCKSIZE;
	}
	return nbytes;
}

int find_mtd_partition_by_name(const char *name)
{
	char *line = NULL;
	size_t len;
	int num = -1;
	FILE *fp;

	if (name == NULL)
		return -1;

	fp = fopen(MTD_PROC_FILENAME, "r");
	if (!fp)
		return -1;

	while (getline(&line, &len, fp) > 0) {
		char mtdname[64];
		int mtdnum, mtdsize, mtderasesize;
		sscanf(line, "mtd%d: %x %x \"%63[^\"]",
				&mtdnum, &mtdsize, &mtderasesize, mtdname);
		if (!strncmp(name, mtdname, strlen(name))) {
				num = mtdnum;
				break;
		}
	}
	fclose(fp);

	return num;
}

int get_bootloader_message(struct bootloader_message *out)
{
	void *buf;
	int bufsize = RKFLASH_BLOCKSIZE*MISC_PAGES;
	char miscpath[128];
	int miscnum, nbytes;
	int miscfd;

	miscnum = find_mtd_partition_by_name("misc");
	if (miscnum < 0) {
		fprintf(stderr, "failed to find misc.\n");
		return -1;
	}
	snprintf(miscpath, sizeof(miscpath), "/dev/mtd%d", miscnum);

	miscfd = open(miscpath, O_RDONLY);
	if (miscfd < 0) {
		fprintf(stderr, "failed to open %s.\n", miscpath);
		return -1;
	}

	buf = malloc(bufsize);
	if (!buf)
		return -1;

	memset(buf, 0, bufsize);

	nbytes = rkflash_read(miscfd, buf, bufsize);
	close(miscfd);
	if (nbytes != bufsize) {
		fprintf(stderr, "failed to read misc.\n");
		return -1;
	}

	memcpy(out, buf + RKFLASH_BLOCKSIZE * MISC_COMMAND_PAGE, sizeof(struct bootloader_message));
	free(buf);

	return 0;
}

int set_bootloader_message(struct bootloader_message *in)
{
	void *buf;
	int bufsize = RKFLASH_BLOCKSIZE*MISC_PAGES;
	char miscpath[128];
	int miscnum, nbytes;
	int miscfd;

	miscnum = find_mtd_partition_by_name("misc");
	if (miscnum < 0) {
		fprintf(stderr, "failed to find misc.\n");
		return -1;
	}
	snprintf(miscpath, sizeof(miscpath), "/dev/mtd%d", miscnum);

	miscfd = open(miscpath, O_RDONLY);
	if (miscfd < 0) {
		fprintf(stderr, "failed to open %s.\n", miscpath);
		return -1;
	}

	buf = malloc(bufsize);
	if (!buf)
		return -1;
	memset(buf, 0, bufsize);

	nbytes = rkflash_read(miscfd, buf, bufsize);
	close(miscfd);
	if (nbytes != bufsize) {
		fprintf(stderr, "failed to read misc.\n");
		return -1;
	}
	memcpy(buf + RKFLASH_BLOCKSIZE * MISC_COMMAND_PAGE, in, sizeof(struct bootloader_message));
	miscfd = open(miscpath, O_RDWR);
	nbytes = rkflash_write(miscfd, buf, bufsize);
	close(miscfd);
	if (nbytes != bufsize) {
		fprintf(stderr, "failed to write misc.\n");
		return -1;
	}
	free(buf);

	return 0;
}

void usage(void)
{
    fprintf(stderr, "Usage:\n"
            "'blmessage' -- Read bootloader message\n"
            "'blmessage -w [-c command -s status -r recovery]' -- write bootloader message\n");
}

int main(int argc, char *argv[])
{
	struct bootloader_message boot;
	const char *command = 0, *status = 0, *recovery = 0;
	int opt, write = 0;

	while ((opt = getopt(argc, argv, "c:s:r:wh")) != -1) {
		switch (opt) {
		case 'c':
			command = strdup(optarg);
			break;
		case 's':
			status = strdup(optarg);
			break;
		case 'r':
			recovery = strdup(optarg);
			break;
		case 'w':
			write = 1;
			break;
        case 'h':
            usage();
            return 0;
        default:
            usage();
            return -1;
		}
	}

	memset(&boot, 0, sizeof(boot));
	get_bootloader_message(&boot);
	if (write) {
		if (command)
			strncpy(boot.command, command, sizeof(boot.command));
		if (status)
			strncpy(boot.status, status, sizeof(boot.status));
		if (recovery)
			strncpy(boot.recovery, recovery, sizeof(boot.recovery));
		set_bootloader_message(&boot);
	} else {
		fprintf(stdout, "command=%s\nstatus=%s\nrecovery=%s\n",
				boot.command, boot.status, boot.recovery);
	}

	return 0;
}
