#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/time.h>

int main(int argc, char *argv[])
{
	int fd,ret;

	if(argc > 4) {
		printf("Usage: ioctl <path> <arg1> [arg2]\n");
		return 0;
	}
	
	fd = open(argv[1],O_RDWR);

	if(fd <= 0) {
		printf("Failed to open %s!\n",argv[1]);
		return -1;
	}

	if (argc == 3)
		ret = ioctl(fd, atoi(argv[2]));
	else
		ret = ioctl(fd, atoi(argv[2]), atoi(argv[3]));

	close(fd);

	return 0;
}
