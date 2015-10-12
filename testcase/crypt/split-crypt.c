#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

int order[] = { 3, 2, 1, 0 };
int blockcount = sizeof(order)/sizeof(order[0]);

struct file_block {
	int fd;
	int addr;
	int size;
};

void usage(void)
{
	fprintf(stderr, "\n");
}

int block_move(struct file_block *ifb, struct file_block *ofb)
{
	int ret;
	int size, bytes;
	void *buf;
	int i, blocks;
	int src, dst;
	int ifd, ofd;

	src = ifb->addr;
	dst = ofb->addr;
	ifd = ifb->fd;
	ofd = ofb->fd;
	size = ifb->size;

	buf = malloc(1024*1024);

	lseek(ifd, src, SEEK_SET);
	lseek(ofd, dst, SEEK_SET);

	blocks = (size+1024*1024-1) / (1024*1024);
	bytes = size % (1024*1024);

	for (i = 1; i < blocks; i++) {
		ret = read(ifd, buf, 1024*1024);
		write(ofd, buf, ret);
	}

	ret = read(ifd, buf, bytes);
	write(ofd, buf, ret);
	printf("src=%d,dst=%d,bytes=%d,blocks=%d\n",src,dst,bytes,blocks);

	return 0;
}

int find_block(int fd, int n, struct file_block *fb)
{
	int addr = 0;
	int size, bsize, lsize;
	int i;

	size = lseek(fd, 0, SEEK_END);
	bsize = size / blockcount;
	lsize = size - (blockcount - 1) * bsize;

	for (i = 0; i < blockcount; i++) {
		if (order[i] == n)
			break;

		addr += (order[i] == (blockcount-1)) ? lsize : bsize;
	}

	fb->addr = addr;
	fb->size = (n == (blockcount-1)) ? lsize : bsize;
	printf("size=%d,addr=%d,size=%d,n=%d\n",size,addr,fb->size,n);

	return 0;
}

int encrypt(int ifd, int ofd)
{
	int size, bsize;
	int src = 0, dst = 0;
	struct file_block ib, ob;
	int addr = 0;
	int i;

	ib.fd = ifd;
	ob.fd = ofd;

	for (i = 0; i < blockcount; i++) {
		struct file_block fb;

		find_block(ifd, i, &fb);

		ob.addr = fb.addr;
		ob.size = fb.size;

		ib.addr = addr;
		ib.size = fb.size;

		addr += fb.size;

		block_move(&ib, &ob);
	}
}

int decrypt(int ifd, int ofd)
{
	int size, bsize;
	int src = 0, dst = 0;
	struct file_block ib, ob;
	int addr = 0, i;

	ib.fd = ifd;
	ob.fd = ofd;

	for (i = 0; i < blockcount; i++) {
		struct file_block fb;

		find_block(ifd, i, &fb);

		ib.addr = fb.addr;
		ib.size = fb.size;

		ob.addr = addr;
		ob.size = fb.size;

		addr += fb.size;

		block_move(&ib, &ob);
	}
}

int main(int argc, char *argv[])
{
	char *p;
	const char *cmdname = argv[0];
	char *ipath, *opath;
	int opt;
	int ifd, ofd;

	if ((p = strrchr(cmdname, '/')) != NULL)
		cmdname = p + 1;

	while ((opt=getopt(argc, argv, "f:o:c:")) != -1) {
		switch (opt) {
			case 'f':
				ipath=optarg;
				printf("input file: %s\n", ipath);
				break;
			case 'o':
				opath=optarg;
				printf("output file: %s\n", opath);
				break;
			case 'c':
				blockcount=atoi(optarg);
				break;
			default:
				usage();
				return -1;
		}
	}

	ifd = open(ipath, O_RDONLY, S_IRUSR|S_IRGRP|S_IROTH);
	if (ifd < 0)
		return -1;

	ofd = open(opath, O_WRONLY|O_CREAT, S_IRWXU);
	if (ofd < 0)
		return -1;

	if (!strcmp(cmdname, "encrypt")) {

		printf("encrypt\n");
		return encrypt(ifd, ofd);

	} else if (!strcmp(cmdname, "decrypt")) {

		printf("decrypt\n");
		return decrypt(ifd, ofd);

	}

	close(ifd);
	close(ofd);

	return 0;
}
