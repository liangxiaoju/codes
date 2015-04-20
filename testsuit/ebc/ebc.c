#include <stdio.h>
#include <stdlib.h>
#include <linux/fb.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>

#define GET_EBC_BUFFER 0x7000
#define SET_EBC_SEND_BUFFER 0x7001
#define GET_EBC_BUFFER_INFO 0x7003

#define EBC_DEV "/dev/ebc"

struct ebc_buf_info{
	int offset;
	int epd_mode;
	int height;
	int width;
	int vir_height;
	int vir_width;
};

struct ebc_dev {
    int ebc_fd;
    int ebc_len;
    void *ebc_mem;
    void *rgb_buf;
    struct ebc_buf_info info;
    int rotate;
};

int ebc_init(struct ebc_dev *dev)
{
    struct ebc_buf_info *info = &dev->info;

    dev->ebc_fd = open(EBC_DEV, O_RDWR);
    if (dev->ebc_fd < 0) {
		printf("no ebc.\n");
        return -1;
    }

    if (ioctl(dev->ebc_fd, GET_EBC_BUFFER_INFO, info) != 0) {
        perror("ioctl");
        return -1;
    }

    dev->ebc_len = info->vir_width * info->vir_height * 2;
    dev->ebc_mem = mmap(0, dev->ebc_len, PROT_READ | PROT_WRITE, MAP_SHARED,
            dev->ebc_fd, 0);
    if (dev->ebc_mem == MAP_FAILED) {
        perror("mmap");
        return -1;
    }

    printf("ebc_init: vwidth=%d vheight=%d\n", info->vir_width, info->vir_height);

    return 0;
}

int ebc_exit(struct ebc_dev *dev)
{
    if (dev->ebc_fd > 0) {
        munmap(dev->ebc_mem, dev->ebc_len);
        close(dev->ebc_fd);
    }
    return 0;
}

int ebc_refresh(struct ebc_dev *dev, void *gray, int mode)
{
	struct ebc_buf_info *info = &dev->info;

	if (dev->ebc_fd <= 0)
		return -1;

    if (ioctl(dev->ebc_fd, GET_EBC_BUFFER, info) != 0) {
        perror("ioctl GET_EBC_BUFFER");
        return -1;
    }

    info->epd_mode = mode;

	memcpy(dev->ebc_mem + info->offset, gray, info->vir_width * info->vir_height / 2);

    if (ioctl(dev->ebc_fd, SET_EBC_SEND_BUFFER, info) != 0) {
        perror("ioctl SET_EBC_SEND_BUFFER");
        return -1;
    }

    return 0;
}

int ebc_clean(struct ebc_dev *dev)
{
	struct ebc_buf_info *info = &dev->info;

	if (dev->ebc_fd <= 0)
		return -1;

    if (ioctl(dev->ebc_fd, GET_EBC_BUFFER, info) != 0) {
        perror("ioctl GET_EBC_BUFFER");
        return -1;
    }

    info->epd_mode = 1;

	memset(dev->ebc_mem + info->offset, 0xff, info->vir_width * info->vir_height / 2);

    if (ioctl(dev->ebc_fd, SET_EBC_SEND_BUFFER, info) != 0) {
        perror("ioctl SET_EBC_SEND_BUFFER");
        return -1;
    }

    return 0;
}

int main(int argc, char *argv[])
{
	struct ebc_dev dev;
	struct stat st;
	time_t t, start, end;
	char **buf;
	int *fd;
	int opt, i, j = 0, num, mode = 1;
	int number = 100;

	while ((opt = getopt(argc, argv, "m:n:")) != -1) {
		switch (opt) {
			case 'm':
				mode = atoi(optarg);
				break;
			case 'n':
				number = atoi(optarg);
				break;
			default:
				return -1;
		}
	}

	num = argc - optind;

	buf = malloc(sizeof(char *) * num);
	fd = malloc(sizeof(int) * num);
	if (!buf || !fd) {
		perror("malloc");
		return -1;
	}

	for (i = 0; i < num; i++) {
		fd[i] = open(argv[optind++], O_RDONLY);
		if (fd[i] <= 0) {
			perror("open");
			return -1;
		}
		if (fstat(fd[i], &st) < 0) {
			perror("fstat");
			return -1;
		}
		buf[i] = malloc(st.st_size);
		if (!buf[i]) {
			perror("malloc");
			return -1;
		}
		if (read(fd[i], buf[i], st.st_size) != st.st_size) {
			perror("read");
			return -1;
		}
		close(fd[i]);
	}

	ebc_init(&dev);
	ebc_clean(&dev);

	start = time(NULL);
	while (1) {
		for (i = 0; i < num; ++i) {
			if (num*j+i+1 > number)
				goto exit;
			//t = time(NULL);
			//printf("%d: %s\n", num*j+i+1, ctime(&t));
			ebc_refresh(&dev, buf[i], mode);
		}
		++j;
	}

exit:
	end = time(NULL);
	printf("summary: %ld/%d %lf\n", end-start, number, (double)(end-start)/number);

	ebc_exit(&dev);

	return 0;
}

