#include <stdio.h> 

#define RGB2Luma_4bit(r,g,b)        ((77*r+150*g+29*b)>>12)

int rgb888_to_gray(unsigned short *luma, unsigned char *src,
        unsigned int bytes)
{
    unsigned int i;
    unsigned int r, g, b, g0, g1, g2, g3;

    for (i = 0; i < bytes; i += 4) {
        r = *src++;
        g = *src++;
        b = *src++;
        g0 = RGB2Luma_4bit(r, g, b);

        r = *src++;
        g = *src++;
        b = *src++;
        g1 = RGB2Luma_4bit(r, g, b);

        r = *src++;
        g = *src++;
        b = *src++;
        g2 = RGB2Luma_4bit(r, g, b);

        r = *src++;
        g = *src++;
        b = *src++;
        g3 = RGB2Luma_4bit(r, g, b);

        *luma++ = (g0 & 0x0F) | ((g1 & 0xF)<<4) |
            ((g2 & 0xF)<<8)| ((g3 & 0xF)<<12);
    }

	return i/6;
}

int main(int argc, char *argv[])
{
	char buf[1024*3];
	char gray[1024/2];
	int bytes;

	while ((bytes = read(0, buf, sizeof(buf))) > 0) {
		bytes = rgb888_to_gray((unsigned short *)gray, (unsigned char *)buf, bytes);
		write(1, gray, bytes);
	}

	return 0;
}
