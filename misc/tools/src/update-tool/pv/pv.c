/*
 * pipe viewer
 * Example: dd if=/dev/zero | pv -t 5 | dd of=/dev/null 
 */

#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char *argv[])
{
	int opt, i;
	int fds[1024];
	int s_step;
	time_t	t0, t1, t2, t_step;
	unsigned long len1, len2, size;
	unsigned long total_size = 0;
	char buf[16*1024];

	memset(fds, ~0, sizeof(fds));
	fds[0] = STDERR_FILENO;
	s_step = 0;
	t_step = 0;

	while ((opt=getopt(argc, argv, "s:t:m:")) != -1) {
		switch (opt) {
			case 's':
				s_step = atoi(optarg);
				break;
			case 't':
				t_step = atoi(optarg);
				break;
			case 'm':
				total_size = atoi(optarg);
				break;
			default:
				fprintf(stderr, "Usage: %s [-s size_step] [-t time_step] file...\n", argv[0]);
				return -1;
		}
	}

	i = 0;
	while (argc > optind) {
		fds[i] = open(argv[optind], O_CREAT|O_RDWR, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
		if (fds[i] < 0) {
			perror("open");
			fprintf(stderr, "Usage: %s [-s size_step] [-t time_step] file...\n", argv[0]);
			return -1;
		}

		if (i > sizeof(fds)/sizeof(fds[0])) {
			fprintf(stderr, "Usage: %s [-s size_step] [-t time_step] file...\n", argv[0]);
			return -1;
		}

		optind++;
		i++;
	}

	if (!s_step && !t_step)
		t_step = 5;

	len1 = len2 = 0;
	t0 = t1 = t2 = time(NULL);
	while ((size = read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
		if (write(STDOUT_FILENO, buf, size) != size) {
			perror("write");
			return -1;
		}

		len2 += size;
		t2 = time(NULL);
		if ((t_step &&t2-t1 >= t_step) || (s_step &&len2 - len1 >= s_step) || (total_size && (len2 >= total_size))) {

			if (!total_size)
				sprintf(buf, "%10u Bytes Completed, eclipsed %d seconds.\n", len2, t2-t0);
			else
				sprintf(buf, "%10u Bytes Completed. %3d%%\r", len2, (len2/(total_size/100)));

			i = 0;
			t1 = t2;
			len1 = len2;
			while (fds[i] > 0) {
				if (write(fds[i], buf, strlen(buf)) < 0) {
					perror("write");
					break;
				}
				i++;
			}

		}
	}

	i = 0;
	while (fds[i] > 0) 
		close(fds[i++]);
	
	return 0;
}
