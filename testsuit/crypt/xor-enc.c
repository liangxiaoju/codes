#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define DEFAULT_KEYWORD	"RK28board"
#define DEFAULT_KEYFILE "/proc/cpuinfo"
#define DEFAULT_KEYWORD_2 "boeye"

void usage(void)
{
	fprintf(stderr, "xor-enc [-k keyfile][-K keyword]\n");
}

int main(int argc, char *argv[])
{
	int i, opt, bytes, kp = 0;
	char *buf;
	int buflen=16*1024;
	char *key = strdup(DEFAULT_KEYWORD);
	int keylen = strlen(DEFAULT_KEYWORD);
	int fd;
	char keyfile[1024*1024];
	char *path;
	char *key2 = strdup(DEFAULT_KEYWORD_2);
	int keylen2 = strlen(DEFAULT_KEYWORD_2);

	buf = malloc(buflen);
	if (!buf)
		return -1;

	while ((opt = getopt(argc, argv, "k:K:d")) != -1) {
		switch (opt) {
			case 'k':
				path = optarg;
				if (!strcmp(optarg, "default"))
					path = DEFAULT_KEYFILE;

				fd = open(path, O_RDONLY, S_IRUSR|S_IRGRP|S_IROTH);
				if (fd <= 0) {
					usage();
					return -1;
				}

				keylen = 1024*1024;
				keylen = read(fd, keyfile, keylen);
				key = keyfile;
				break;
			case 'K':
				key = strdup(optarg);
				keylen = strlen(optarg);
				break;
			default:
				usage();
				return -1;
		}
	}

	kp = 0;
	for (i = 0; i < keylen; i++) {

		kp = kp % keylen2;

		key[i] ^= *(key2 + kp);

		kp++;
	}

	kp = 0;
	while ((bytes = read(0, buf, buflen)) > 0) {

		for (i = 0; i < bytes; i++) {

			kp = kp % keylen;

			buf[i] ^= *(key + kp);

			kp++;
		}

		write(1, buf, bytes);
	}

	return 0;
}
