/*
 * Decode sibrary.img
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/sendfile.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


struct SibUpdate
{
	char flag[32];
	int version;
	int size;
};

struct Package
{
	char module[4];
	int version;
	int size;
	/* int offset; */
};

int extract(int in_fd, char *file, int len)
{
	int out_fd;
	
	out_fd = open(file, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	if (out_fd < 0) {
		perror("Open error");
		return -1;
	}
	if (sendfile(out_fd, in_fd, NULL, len) < 0) {
		perror("sendfile error");
		return -1;
	}

	close(out_fd);
	
	return 0;
}

int main(int argc, const char *argv[])
{
	int fd;
	int opt;
	int dirlen = 0;
	char module[1024];
	struct SibUpdate update;
	struct Package package;

	if (argc < 2) {
		fprintf(stderr, "Parameter error\n");
		fprintf(stderr, "Usage: %s [-C path] image\n", argv[0]);
		return -1;
	}

	memset(module, 0, sizeof(module));
	if ((opt=getopt(argc, argv, "C:")) != -1) {
		switch (opt) {
			case 'C':
				strncpy(module, optarg, sizeof(module)-1);
				module[strlen(module)] = '/';	/* Directory delimiter */
				dirlen = strlen(module);
				break;
			default:
				fprintf(stderr, "Parameter error\n");
				return -1;
		}
	}

	fd = open(argv[optind], O_RDONLY);
	if (fd < 0) {
		perror("Open error");
		return -1;
	}
	
	if (read(fd, &update, sizeof(update)) != sizeof(update)) {
		fprintf(stderr, "Read error\n");
		return -1;
	}
	if (strncmp(update.flag, "boeyesibraryupdate", 18) != 0) {
		fprintf(stderr, "%s is NOT valid\n", argv[1]);
		return -1;
	}
	
	strcat(module, "update");	/* not safe */
	extract(fd, module, update.size);

	while (read(fd, &package, sizeof(package)) == sizeof(package)) {
		memset(module+dirlen, 0, sizeof(module)-dirlen);
		strncat(module, package.module, sizeof(package.module));

		printf("%s: version(0x%x)\n", module, package.version);
		extract(fd, module, package.size);
	}

	return 0;
}
