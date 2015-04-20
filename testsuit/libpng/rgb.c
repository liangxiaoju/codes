#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

inline unsigned short int rgb_24_2_565(unsigned int r, unsigned int g, unsigned int b)
{  
	return  (unsigned short int)((((r) << 8) & 0xF800) | (((g) << 3) & 0x7E0) | ((b) >> 3));
}

int to_rgb_565(unsigned char *dst, unsigned char *src, int bytes)
{
	int i;

	for (i = 0; i < bytes; i += 3) {
		*(((unsigned short int *)dst + (i/3))) = rgb_24_2_565(src[i], src[i+1], src[i+2]);
	}
	return i/3 * 2;
}

int main(int argc, const char *argv[])
{
	int cnt = 800;
	unsigned char src[3*cnt];
	unsigned char dst[2*cnt];
	int src_bytes, dst_bytes;

	while ((src_bytes = read(STDIN_FILENO, src, sizeof(src))) > 0) {
		dst_bytes = to_rgb_565(dst, src, src_bytes);
		write(STDOUT_FILENO, dst, dst_bytes);
	}

	return 0;
}
