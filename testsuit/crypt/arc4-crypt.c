#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define ARC4_BLOCK_SIZE		(16*1024)
#define ARC4_S_BOX_SIZE		(256)
#define ARC4_MAX_KEY_SIZE	(256)

typedef unsigned char u8;

struct arc4_ctx {
	u8 S[ARC4_S_BOX_SIZE];
	u8 x, y;
};

static int arc4_set_key(struct arc4_ctx *ctx, const u8 *in_key, unsigned int key_len)
{
	int i, j = 0, k = 0;

	ctx->x = 1;
	ctx->y = 0;

	for (i = 0; i < ARC4_S_BOX_SIZE; i++)
		ctx->S[i] = i;

	for (i = 0; i < ARC4_S_BOX_SIZE; i++) {
		u8 a = ctx->S[i];
		j = (j + in_key[k] + a) & 0xff;
		ctx->S[i] = ctx->S[j];
		ctx->S[j] = a;
		if (++k >= key_len)
			k = 0;
	}

	return 0;
}

static void arc4_crypt(struct arc4_ctx *ctx, u8 *data, unsigned int data_len)
{
	u8 *const S = ctx->S;
	u8 x = ctx->x;
	u8 y = ctx->y;
	u8 a, b;
	unsigned int i;

	for (i = 0; i < data_len; i++) {
		a = S[x];
		y = (y + a) & 0xff;
		b = S[y];
		S[x] = b;
		S[y] = a;
		x = (x + 1) & 0xff;
		*data++ ^= S[(a + b) & 0xff];

		ctx->x = x;
		ctx->y = y;
	}
}

static void Usage(void)
{
	fprintf(stderr, "crypt < [-k keyfile] [-K keyword] >\n");
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
	char key[ARC4_MAX_KEY_SIZE], data[ARC4_BLOCK_SIZE];
	int key_len;
	struct arc4_ctx ctx;
	int opt, nbytes;
	int fd;

	if (argc < 2)
		Usage();

	while ((opt = getopt(argc, argv, "k:K:")) != -1) {
		switch (opt) {
			case 'k':
				fd = open(optarg, O_RDONLY);
				if (fd < 0)
					Usage();
				key_len = read(fd, key, ARC4_MAX_KEY_SIZE);
				if (key_len <= 0)
					Usage();
				close(fd);
				break;
			case 'K':
				key_len = strlen(optarg);
				key_len = (key_len > ARC4_MAX_KEY_SIZE) ? ARC4_MAX_KEY_SIZE: key_len;
				memcpy(key, optarg, key_len);
				break;
			default:
				Usage();
		}
	}

	arc4_set_key(&ctx, key, key_len);

	while ((nbytes = read(STDIN_FILENO, data, ARC4_BLOCK_SIZE)) > 0) {

		arc4_crypt(&ctx, data, nbytes);

		write(STDOUT_FILENO, data, nbytes);

	}

	return 0;
}
