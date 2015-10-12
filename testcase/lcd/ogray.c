#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <getopt.h>
#include <string.h>

#define width	(1024)
#define height	(768)

int main(int argc, char *argv[])
{
	int i, j, c;
	char opath[256];
	unsigned char buf[height*width*4] = {0};
	unsigned char gray = 0;
	int fd;

	while (c = getopt(argc, argv, "o:n") != -1) {
		printf("optarg=%x", c);
		switch(c) {
		case 'o':
			strcpy(opath, optarg);
			break;
		default:
			printf("Error arg.\n");
			break;
		}
	}

	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			buf[i*width + j*4 + 0] = 0;
			buf[i*width + j*4 + 1] = gray;
			buf[i*width + j*4 + 2] = gray;
			buf[i*width + j*4 + 3] = gray;
		}
		gray ++;
	}

	fd = open(opath, O_RDWR|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IROTH);
	if (fd < 0) {
		printf("Failed to open %s.\n", opath);
		return -1;
	}

	write(fd, buf, height*width*4);

	close(fd);

	return 0;
}
