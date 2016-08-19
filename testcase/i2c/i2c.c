#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>

void usage(const char *name)
{
	printf("Usage: %s -d DEV -a ADDR -r LEN -w 'DATA'\n", name);
	printf("\t-d DEV -- i2c-dev, eg. /dev/i2c-0\n");
	printf("\t-a ADDR -- i2c device address\n");
	printf("\t-r LEN -- LEN bytes to read\n");
	printf("\t-w 'DATA' -- DATA to write\n");
}

int main(int argc, char *argv[])
{
	char *action;
	int addr = 0;
	char *dev = NULL;
	int fd;
    int opt;
	int rlen = 0, wlen = 0;
	char *rbuf, wbuf[1024];
	int i, ret;

    while ((opt = getopt(argc, argv, "d:a:r:w:h")) != -1) {
        switch (opt) {
            case 'd':
                dev = strdup(optarg);
                break;
            case 'a':
				addr = strtol(optarg, NULL, 0);
                break;
			case 'r':
				rlen = strtol(optarg, NULL, 0);
				break;
			case 'w':
				i = 0;
				char *t = strtok(optarg, ", ");
				while (t && i < sizeof(wbuf)) {
					wbuf[i++] = strtol(t, (char **) NULL, 0);
					t = strtok(NULL, ", ");
				}
				wlen = i;
				break;
            case 'h':
            default:
                usage(argv[0]);
                return 0;
        }
    }

	if (!dev || !addr) {
		usage(argv[0]);
		return 0;
	}

	printf("DEV: %s, ADDR: 0x%x\n", dev, addr);

	fd = open(dev, O_RDWR);
	if (fd < 0) {
		perror("open");
		return -1;
	}

	ret = ioctl(fd, I2C_SLAVE_FORCE, addr);
	if (ret < 0) {
		perror("ioctl");
		return -1;
	}

	if (rlen > 0) {
		char *rbuf = malloc(rlen);
		if (!rbuf) {
			perror("malloc");
			return -1;
		}
		memset(rbuf, 0, rlen);

		ret = read(fd, rbuf, rlen);
		if (ret < 0) {
			perror("read");
			return -1;
		}

		printf("Read:");
		for (i = 0; i < rlen; i++) {
			printf(" 0x%x", rbuf[i]);
		}
		printf("\n");

		free(rbuf);
	} else if (wlen > 0) {
		ret = write(fd, wbuf, wlen);
		if (ret < 0) {
			perror("write");
			return -1;
		}

		printf("Write:");
		for (i = 0; i < wlen; i++) {
			printf(" 0x%x", wbuf[i]);
		}
		printf("\n");
	}

	close(fd);

	return 0;
}
