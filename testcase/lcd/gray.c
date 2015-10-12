#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#define width	(1024)
#define height	(768)
#define bpp	(32)
#define opath	"/tmp/data"

int main()
{
	int i, j;
	unsigned char buf[height*width*(bpp/8)] = {0};
	unsigned char gray = 0;
	int fd;

	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			buf[i*width*(bpp/8) + j*(bpp/8) + 0] = 0;//blue
			buf[i*width*(bpp/8) + j*(bpp/8) + 1] = 0;//green
			buf[i*width*(bpp/8) + j*(bpp/8) + 2] = 255;//red
			buf[i*width*(bpp/8) + j*(bpp/8) + 3] = 0;
		}
		if (!(i % (height/256)))
			gray ++;
	}

	fd = open(opath, O_RDWR|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IROTH);
	if (fd < 0) {
		printf("Failed to open %s.\n", opath);
		return -1;
	}

	write(fd, buf, height*width*(bpp/8));

	close(fd);

	return 0;
}
