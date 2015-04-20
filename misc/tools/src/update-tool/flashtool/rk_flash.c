#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#define PAGE_SIZE (512)
#define BLOCK_PAGE (32)
#define page_to_byte(x) (x*PAGE_SIZE)
#define byte_to_page(x) (((x)+PAGE_SIZE-1)/PAGE_SIZE)
#define page_to_block(x) (((x)+BLOCK_PAGE-1)/BLOCK_PAGE)

void usage(void)
{
	fprintf(stderr, "Usage:\n"
			"rk_flash -r pages -f device [-s pages] > file \n"
			"rk_flash -w -f device [-s pages] < file\n"
		   );
}

int main(int argc, char *argv[])
{
	int fd, size, seekpage = 0, opt, readpage = 0;
	char *buf;
	char const *file;

	while ((opt = getopt(argc, argv, "r:wf:o:s:h")) != -1) {
		switch (opt) {
		case 'r':
			readpage = strtol(optarg, NULL, 0);
			break;
		case 'w':
			readpage = 0;
			break;
		case 'f':
			file = strdup(optarg);
			break;
		case 's':
			seekpage = strtol(optarg, NULL, 0);
			break;
		case 'h':
			usage();
			return 0;
		}
	}

	buf = malloc(page_to_byte(BLOCK_PAGE));
	if (!buf) {
		fprintf(stderr, "failed to malloc.\n");
		return -1;
	}

	if (readpage)
		fd = open(file, O_RDONLY);
	else
		fd = open(file, O_WRONLY|O_CREAT);

	if (fd < 0) {
		fprintf(stderr, "failed to open %s.\n", file);
		return -1;
	}

	if (lseek(fd, seekpage, SEEK_SET) < 0) {
		fprintf(stderr, "failed to lseek.\n");
		return -1;
	}

	/* XXX we read or write one block every time */
	if (readpage) {
		int blocks = page_to_block(readpage);
		while (blocks--) {
			memset(buf, 0, page_to_byte(BLOCK_PAGE));
			size = read(fd, buf, BLOCK_PAGE);
			size = page_to_byte(size);
			write(STDOUT_FILENO, buf, size);
		}
	} else {
		int page;
		while ((size = read(STDIN_FILENO, buf, page_to_byte(BLOCK_PAGE))) > 0) {
			page = byte_to_page(size);
			if (write(fd, buf, page) != page) {
				fprintf(stderr, "failed to write.\n");
				return -1;
			}
			memset(buf, 0, page_to_byte(BLOCK_PAGE));
		}
	}

	close(fd);

	return 0;
}
