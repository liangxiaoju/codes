#include <stdio.h>
#include <stdlib.h>
#include <linux/fb.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>

#define COLOR_BLACK	0
#define COLOR_WHITE	~0

#define FB_DEV	"/dev/fb0"

#include "font_8x16.c"
#include "font_sun12x22.c"

const struct font_desc *fonts[] = {
	&font_sun_12x22,
	&font_vga_8x16,
};

struct fb_dev {
	int fd;
	struct fb_var_screeninfo fb_var;
	int row;
	int col;
	void *fbmem;
	int fblen;
	const struct font_desc *font;
};

int write_fb(struct fb_dev *, unsigned char *buf);
int write_ch(struct fb_dev *fb_dev, int row, int col, unsigned char ch);

const struct font_desc *find_font(const char *name)
{
	int i;

	for (i = 0; i < sizeof(fonts)/sizeof(fonts[0]); i++) {
		if (!strcmp(fonts[i]->name, name))
			return fonts[i];
	}
	return NULL;
}

int main(int argc, char *argv[])
{
	int opt;
	int infile;
	char buf[1024];
	struct fb_dev fb_dev;

	infile = STDIN_FILENO;
	fb_dev.font = fonts[0];
	while ((opt = getopt(argc, argv, "i:o:f:")) != -1) {
		switch (opt) {
			case 'i':
				infile = open(optarg, O_RDONLY);
				if (infile < 0)
					perror("open");
				break;
			case 'o':
				fb_dev.fd = open(optarg, O_RDWR);
				if (fb_dev.fd < 0)
					perror("open");
				break;
			case 'f':
				fb_dev.font = find_font(optarg);
				if (!fb_dev.font)
					return -1;
				break;
			default:
				fprintf(stderr, "Usage: %s [-i fifo] [-o /dev/fb0]\n", argv[0]);
				return -1;
		}
	}

	if (fb_dev.fd<=0) {
		fb_dev.fd = open(FB_DEV, O_RDWR);
		if (fb_dev.fd < 0)
			perror("open");
	}

	if (ioctl(fb_dev.fd, FBIOGET_VSCREENINFO, &fb_dev.fb_var) < 0) {
		perror("ioctl");
		return -1;
	}

	printf("xres: %d, yres: %d, bpp: %d\n", fb_dev.fb_var.xres, fb_dev.fb_var.yres, fb_dev.fb_var.bits_per_pixel);
	
	fb_dev.col = fb_dev.fb_var.xres/fb_dev.font->width;
	fb_dev.row = fb_dev.fb_var.yres/fb_dev.font->height;
	fb_dev.fblen = fb_dev.fb_var.xres * fb_dev.fb_var.yres * fb_dev.fb_var.bits_per_pixel/8;
	fb_dev.fbmem = mmap(NULL, fb_dev.fblen, PROT_READ|PROT_WRITE, MAP_SHARED, fb_dev.fd, 0);
	if (!fb_dev.fbmem) {
		perror("mmap");
		return -1;
	}

	memset(fb_dev.fbmem, COLOR_WHITE, fb_dev.fblen);

	while (1) {
		memset(buf, 0, sizeof(buf));
		if (read(infile, buf, sizeof(buf)) < 0) {
			perror("read");
			break;
		}

		if (write_fb(&fb_dev, buf) < 0)
			break;
	}

	munmap(fb_dev.fbmem, fb_dev.fblen);
	close(fb_dev.fd);

	return 0;
}

int write_fb(struct fb_dev *fb_dev, unsigned char *buf)
{
	static int row, col;
	int skip;

	if (!*buf) 
		return 0;

	if (row >= fb_dev->row) {
		memset(fb_dev->fbmem, COLOR_WHITE, fb_dev->fblen);
		row = 0;
		col = 0;
	}

	while (*buf) {
		skip = 1;
		switch (*buf) {
			case '\n':
				row++;
				col = 0;
				break;
			case '\r':
				col = 0;
				break;
			case '\b':
				col--;
				break;
			case '\t':
				col += 8;
				break;
			default:
				skip = 0;
		}

		if (col >= fb_dev->col) {
			row++;
			col = 0;
		}

		if (!skip) {
			write_ch(fb_dev, row, col, *(unsigned char *)buf);
			col++;
		}

		buf++;
	}

#if __arm__
	/* rk2818 eink refresh control */
	if (ioctl(fb_dev->fd, FBIOPAN_DISPLAY, &fb_dev->fb_var) < 0) {
		perror("ioctl");
	}
#endif

	return 0;
}

int write_ch(struct fb_dev *fb_dev, int row, int col, unsigned char ch)
{
	int i, j, k;
	int offset;
	int color = 0;
	char *buf = fb_dev->fbmem;
	int bpp = fb_dev->fb_var.bits_per_pixel/8;
	char *font;
	int height, width, width_bytes;

	height = fb_dev->font->height;
	width = fb_dev->font->width;
	width_bytes = (width + (8 - 1)) / 8;

	offset = row*height*fb_dev->fb_var.xres + col*width;

	font = malloc(width_bytes);
	if (!font) {
		perror("malloc");
		return -1;
	}

	for (i = 0; i < height; i++) {
		memcpy(font, fb_dev->font->data+(ch*(width_bytes*height)+i*width_bytes), width_bytes);
		for (j = 0; j < width; j++) {
			k = j / 8;
			if (font[k] & 1<<(((7-j)+8*width_bytes)%8))
				memcpy(buf+(offset+i*fb_dev->fb_var.xres+j)*bpp, &color, bpp);
			else
				memset(buf+(offset+i*fb_dev->fb_var.xres+j)*bpp, ~color, bpp);
		}
	}

	free(font);

	return 0;
}
