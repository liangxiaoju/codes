#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>
#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define INPUT_DIR "/dev/input"

char *get_input_path(char *dev_name)
{
    int fd;
	char *dev_path;
	char buf[64];
	DIR *dirp;
	struct dirent *dirent;
	int length, ret;
    struct stat sb;

    /* device path */
    if (stat(dev_name, &sb) != -1) {
        if (S_ISCHR(sb.st_mode)) {
            dev_path = dev_name;
            goto out;
        }
    }

    /* device filename */
    length = strlen(INPUT_DIR)+strlen(dev_name)+2;
    dev_path = realloc(NULL, length);
    snprintf(dev_path, length, "%s/%s", INPUT_DIR, dev_name);
    if (stat(dev_path, &sb) != -1) {
        if (S_ISCHR(sb.st_mode)) {
            goto out;
        }
    }

    /* inputdev name */
	dirp = opendir(INPUT_DIR);
	if (dirp == NULL) {
        perror("opendiri");
        return NULL;
    }
	for (;;) {
		dirent = readdir(dirp);
		if (dirent == NULL) {
			ret = -1;
			break;
		}

		if (dirent->d_type == DT_CHR) {
			length = strlen(dirent->d_name);
			length += sizeof(INPUT_DIR) + 1;
			dev_path = malloc(length);
			if (dev_path == NULL) {
                perror("malloc");
                return NULL;
            }
			snprintf(dev_path, length,"%s/%s",
                     INPUT_DIR, dirent->d_name);

			fd = open(dev_path, O_RDONLY);
			if (fd <= 0) {
                perror("open");
                return NULL;
            }
			ioctl(fd, EVIOCGNAME(64), buf);
            close(fd);

			if (strcmp(dev_name, buf))
				continue;

            break;
		}
	}
    closedir(dirp);

out:
    fprintf(stdout, "Found: %s <--> %s\n", dev_name, dev_path);

    return dev_path;
}

int read_input(const char *path)
{
    int fd;
	struct input_event event;

    fd = open(path, O_RDONLY);
    if (fd <= 0) {
        perror("open");
        return -1;
    }

    while (1) {
        read(fd, &event, sizeof(struct input_event));
        fprintf(stdout, "Time:%lld Type:%u Code:%u Value:%d\n",
                (unsigned long long)event.time.tv_sec * 1000 * 1000 + event.time.tv_usec,
                event.type, event.code, event.value);
    }

    close(fd);
    return 0;
}

int main(int argc, char *argv[])
{
	char *dev_path;

	if (argc < 2) {
		fprintf(stderr, "Usage: %s <name>/<path>/<filename>\n", argv[0]);
		return -1;
	}

    dev_path = get_input_path(argv[1]);
    if (!dev_path) {
        fprintf(stderr, "Failed to get input path\n");
        return -1;
    }
    read_input(dev_path);

    free(dev_path);

	return 0;
}
