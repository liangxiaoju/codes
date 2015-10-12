/* ulogo.c - display rootfs logo */

#include <linux/errno.h>
#include <linux/syscalls.h>
#include <linux/fb.h>
#include <linux/zlib.h>
#include <linux/vmalloc.h>
#include <linux/kthread.h>
#include <linux/decompress/inflate.h>
#include "libpng/png.h"

#define ULOGO_VERSION "version 1.0"

struct fb_dev {
    int fd;
    struct fb_var_screeninfo var;
    struct fb_fix_screeninfo fix;
    char __iomem *addr;
    int len;
    char *logo;
};

#ifdef CONFIG_IMT_PNG_LOGO

#define PACKRGB16(r,g,b) (unsigned short) ((((b) & 0xF8) << 8 ) | (((g) & 0xFC) << 3 ) | (((r) & 0xF8) >> 3 ))

void read_png_logo(png_structp png_ptr, png_bytep data, png_size_t length)
{
    png_size_t check;
    int fd = (int)png_get_io_ptr(png_ptr);

    if (png_ptr == NULL)
        return;

    check = sys_read(fd, data, length);
    if (check != length)
        printk(KERN_ERR "[ulogo] ERR: failed to read png logo.\n");
}

void png_error_fn(png_structp png_ptr, png_const_charp msg)
{
    printk(KERN_ERR "[ulogo] ERR: %s", msg);

    /*TODO: how to exit ?*/
    do_exit(-1);
}

void to_rgb32(unsigned char *dst, unsigned char **src, int height, int width, int step)
{
    int h, w;
    int r, g, b;

    for (h = 0; h < height; h++) {
        for (w = 0; w < width; w++) {
            int sx = w*step;
            int dx = h*width*4 + w*4;

            b = src[h][sx+0];
            g = src[h][sx+1];
            r = src[h][sx+2];

            dst[dx+0] = r;
            dst[dx+1] = g;
            dst[dx+2] = b;
            dst[dx+3] = 0xff;
        }
    }
}

void to_rgb16(uint16_t *dst, unsigned char **src, int height, int width, int step)
{
    int h, w;
    int r, g, b;

    for (h = 0; h < height; h++) {
        for (w = 0; w < width; w++) {
            int sx = w*step;
            int dx = h*width*2 + w*2;

            b = src[h][sx+0];
            g = src[h][sx+1];
            r = src[h][sx+2];

            dst[dx/2] = PACKRGB16(r, g, b);
        }
    }
}

int fb_draw_png_ulogo(char *ulogo, void *screen_addr, int screen_len, int x, int y, int bpp)
{
	png_structp png_ptr;
	png_infop info_ptr;
	png_bytepp row_pointers;
    int height, width, channels, bit_depth, color_type, row_size;
    int fd;

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, png_error_fn, NULL);
	info_ptr = png_create_info_struct(png_ptr);

    png_set_crc_action(png_ptr, PNG_CRC_QUIET_USE, PNG_CRC_QUIET_USE);

    fd = sys_open(ulogo, O_RDONLY, 0);
    if (fd < 0) {
	    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        printk(KERN_ERR "Failed to open %s\n", ulogo);
        return -ENOENT;
    }

    png_set_read_fn(png_ptr, (void *)fd, read_png_logo);

	png_read_png(png_ptr, info_ptr,
            PNG_TRANSFORM_IDENTITY |
            PNG_TRANSFORM_SCALE_16 |
            PNG_TRANSFORM_EXPAND |
            PNG_TRANSFORM_GRAY_TO_RGB
            , NULL);

	height = png_get_image_height(png_ptr, info_ptr);
    width = png_get_image_width(png_ptr, info_ptr);
    bit_depth = png_get_bit_depth(png_ptr, info_ptr);
    channels = png_get_channels(png_ptr, info_ptr);
    color_type = png_get_color_type(png_ptr, info_ptr);
	row_size = png_get_rowbytes(png_ptr, info_ptr);
	row_pointers = png_get_rows(png_ptr, info_ptr);

    if ((height != y) || (width != x)) {
        printk(KERN_ERR "Framebuffer[%dx%d] != LOGO[%dx%d]\n", x, y, width, height);
        return -EINVAL;
    }

    switch (color_type) {
        case PNG_COLOR_TYPE_RGBA:
            if (bpp == 16)
                to_rgb16((uint16_t *)screen_addr, row_pointers, height, width, 4);
            else if (bpp == 32)
                to_rgb32((uint8_t *)screen_addr, row_pointers, height, width, 4);
            break;
        case PNG_COLOR_TYPE_RGB:
        case PNG_COLOR_TYPE_PALETTE:
        case PNG_COLOR_TYPE_GRAY:
            if (bpp == 16)
                to_rgb16((uint16_t *)screen_addr, row_pointers, height, width, 3);
            else if (bpp == 32)
                to_rgb32((uint8_t *)screen_addr, row_pointers, height, width, 3);
            break;
        default:
            printk(KERN_ERR "unsupport png color type!\n");
    }

    printk(KERN_INFO "ulogo(png) ok.\n");

	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    sys_close(fd);

    return 0;
}

#endif //CONFIG_IMT_PNG_LOGO

int fb_draw_ulogo(char *ulogo, void *screen_addr, int screen_len, int x, int y, int bpp)
{
    int fd, len;
    char *input;
    struct stat st;

    fd = sys_open(ulogo, O_RDONLY, 0);
    if (fd < 0) {
        printk(KERN_ERR "Failed to open %s\n", ulogo);
        return -ENOENT;
    }

    sys_newfstat(fd, &st);
    len = st.st_size;

    input = vmalloc(len);
    if (!input) {
        printk(KERN_ERR "Failed to vmalloc.\n");
        sys_close(fd);
        return -ENOMEM;
    }

    sys_read(fd, input, len);
    /*
     * TODO: make sure logo_size <= screen_len
     */
    gunzip(input, len, NULL, NULL, screen_addr, NULL, NULL);

    printk(KERN_INFO "ulogo ok.\n");

    vfree(input);
    sys_close(fd);

    return 0;
}

int show_userspace_logo(void *unused)
{
    int err = 0;
    const char *fb_path = "/dev/fb0";
    char logo_path[128];
    struct fb_dev fbdev;

    printk(KERN_INFO "Userspace Logo: %s\n", ULOGO_VERSION);

    memset(&fbdev, 0, sizeof(struct fb_dev));

    err = sys_mknod(fb_path, S_IFCHR|0600, new_encode_dev(MKDEV(FB_MAJOR, 0)));
    if (err < 0) {
        printk(KERN_ERR "Failed to mknod %s\n", fb_path);
        goto out;
    }

    fbdev.fd = sys_open(fb_path, O_RDWR, 0);
    if (fbdev.fd < 0) {
        printk(KERN_ERR "Failed to open %s\n", fb_path);
        goto out;
    }

    if (sys_ioctl(fbdev.fd, FBIOGET_VSCREENINFO, (long)&fbdev.var) < 0) {
        printk(KERN_ERR "Failed to get var info from%s\n", fb_path);
        goto out;
    }

    if (sys_ioctl(fbdev.fd, FBIOGET_FSCREENINFO, (long)&fbdev.fix) < 0) {
        printk(KERN_ERR "Failed to get fix info from %s\n", fb_path);
        goto out;
    }

    fbdev.addr = ioremap(fbdev.fix.smem_start, fbdev.fix.smem_len);
    fbdev.len = fbdev.fix.smem_len;

    sprintf(logo_path,"/%dx%d.gz", fbdev.var.xres, fbdev.var.yres);
    err = fb_draw_ulogo(logo_path, fbdev.addr, fbdev.len, fbdev.var.xres, fbdev.var.yres, fbdev.var.bits_per_pixel);

#ifdef CONFIG_IMT_PNG_LOGO
    if (err) {
        printk(KERN_INFO "Try png logo.\n");
        sprintf(logo_path,"/%dx%d.png", fbdev.var.xres, fbdev.var.yres);
        fb_draw_png_ulogo(logo_path, fbdev.addr, fbdev.len, fbdev.var.xres, fbdev.var.yres, fbdev.var.bits_per_pixel);
    }
#endif //CONFIG_IMT_PNG_LOGO

out:
    if (fbdev.addr)
        iounmap(fbdev.addr);

    if (fbdev.fd > 0)
        sys_close(fbdev.fd);

    sys_unlink(fb_path);

    return 0;
}

static int __init userspace_logo_init(void)
{
    kthread_run(show_userspace_logo, NULL, "ulogo");
    return 0;
}

rootfs_initcall(userspace_logo_init);

MODULE_AUTHOR("liangxiaoju <liangxiaoju@imtechnology.com.cn>");
MODULE_DESCRIPTION("A module used to display boot-logo located in rootfs");
MODULE_LICENSE("GPL");
