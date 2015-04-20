#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <linux/videodev2.h>

#include "fb.h"

int init_fb(struct fb_dev *dev)
{
    dev->fd = open(dev->dev, O_RDWR);
    if (dev->fd < 0) {
        perror("open");
        return -1;
    }

    if (ioctl(dev->fd, FBIOGET_VSCREENINFO, &dev->var) < 0) {
        perror("ioctl FBIOGET_VSCREENINFO");
        return -1;
    }

	printf("xres: %d, yres: %d, bpp: %d\n",
            dev->var.xres, dev->var.yres, dev->var.bits_per_pixel);

    dev->fblen = dev->var.xres * dev->var.yres * dev->var.bits_per_pixel / 8;

    dev->fbmem = mmap(NULL, dev->fblen, PROT_READ|PROT_WRITE, MAP_SHARED, dev->fd, 0);
    if (dev->fbmem == MAP_FAILED) {
        perror("mmap");
        return -1;
    }

    dev->tmpbuf = malloc(dev->fblen);
}

void post_fb(struct fb_dev *dev, void *input, int w, int h, int size, int input_fmt)
{
    int src_bpp;
    int dst_bpp = dev->var.bits_per_pixel / 8;
    void *dst, *src;
    int i, j;

    if (dev->fd < 0)
        return ;

    /* only support YUV422 now */
    if (input_fmt != V4L2_PIX_FMT_YUYV)
        return ;

    switch (dev->var.bits_per_pixel) {
        case 32:
            Pyuv422torgb32(input, dev->tmpbuf, w, h);
            src_bpp = 4;
        case 24:
            Pyuv422torgb24(input, dev->tmpbuf, w, h);
            src_bpp = 3;
            break;
        case 16:
            Pyuv422torgb16(input, dev->tmpbuf, w, h);
            src_bpp = 2;
            break;
        default:
            return ;
    }

    for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++) {
            dst = dev->fbmem + i * dev->var.xres * dst_bpp + j * dst_bpp;
            src = dev->tmpbuf + i * w * src_bpp + j * src_bpp;
            memcpy(dst, src, src_bpp);
        }
    }
}
