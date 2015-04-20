#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <png.h>
#include <linux/fb.h>
#include <stdint.h>

typedef int8_t			GGLbyte;		// b
typedef int16_t			GGLshort;		// s
typedef int32_t			GGLint;			// i
typedef ssize_t			GGLsizei;		// i
typedef int32_t			GGLfixed;		// x
typedef int32_t			GGLclampx;		// x
typedef float			GGLfloat;		// f
typedef float			GGLclampf;		// f
typedef double			GGLdouble;		// d
typedef double			GGLclampd;		// d
typedef uint8_t			GGLubyte;		// ub
typedef uint8_t			GGLboolean;		// ub
typedef uint16_t		GGLushort;		// us
typedef uint32_t		GGLuint;		// ui
typedef unsigned int	GGLenum;		// ui
typedef unsigned int	GGLbitfield;	// ui
typedef void			GGLvoid;
typedef int32_t         GGLfixed32;
typedef	int32_t         GGLcolor;
typedef int32_t         GGLcoord;

typedef struct {
    GGLsizei    version;    // always set to sizeof(GGLSurface)
    GGLuint     width;      // width in pixels
    GGLuint     height;     // height in pixels
    GGLint      stride;     // stride in pixels
    GGLubyte*   data;       // pointer to the bits
    GGLubyte    format;     // pixel format
    GGLubyte    rfu[3];     // must be zero
    // these values are dependent on the used format
    union {
        GGLint  compressedFormat;
        GGLint  vstride;
    };
    void*       reserved;
} GGLSurface;

struct fb_dev {
    int fd;

    struct fb_var_screeninfo var;

    void *fbmem;
    int fblen;

    GGLSurface *pSurface;
};

int res_create_surface(const char* name, GGLSurface** pSurface) {
    char resPath[256];
    GGLSurface* surface = NULL;
    int result = 0;
    unsigned char header[8];
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;

    *pSurface = NULL;

    snprintf(resPath, sizeof(resPath)-1, "%s", name);
    resPath[sizeof(resPath)-1] = '\0';
    FILE* fp = fopen(resPath, "rb");
    if (fp == NULL) {
        perror("fopen");
        return -1;
    }

    size_t bytesRead = fread(header, 1, sizeof(header), fp);
    if (bytesRead != sizeof(header)) {
        result = -2;
        goto exit;
    }

    if (png_sig_cmp(header, 0, sizeof(header))) {
        result = -3;
        goto exit;
    }

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        result = -4;
        goto exit;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        result = -5;
        goto exit;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        result = -6;
        goto exit;
    }

    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, sizeof(header));
    png_read_info(png_ptr, info_ptr);

    int color_type = png_get_color_type(png_ptr, info_ptr);
    int bit_depth = png_get_bit_depth(png_ptr, info_ptr);
    int channels = png_get_channels(png_ptr, info_ptr);
    if (!(bit_depth == 8 &&
          ((channels == 3 && color_type == PNG_COLOR_TYPE_RGB) ||
           (channels == 4 && color_type == PNG_COLOR_TYPE_RGBA) ||
           (channels == 1 && (color_type == PNG_COLOR_TYPE_PALETTE ||
                              color_type == PNG_COLOR_TYPE_GRAY))))) {
        return -7;
        goto exit;
    }

    size_t width = png_get_image_width(png_ptr, info_ptr);
    size_t height = png_get_image_height(png_ptr, info_ptr);
    size_t stride = (color_type == PNG_COLOR_TYPE_GRAY ? 1 : 4) * width;
    size_t pixelSize = stride * height;

    surface = malloc(sizeof(GGLSurface) + pixelSize);
    if (surface == NULL) {
        result = -8;
        goto exit;
    }
    unsigned char* pData = (unsigned char*) (surface + 1);
    surface->version = sizeof(GGLSurface);
    surface->width = width;
    surface->height = height;
    surface->stride = width; /* Yes, pixels, not bytes */
    surface->data = pData;

    int alpha = 0;
    if (color_type == PNG_COLOR_TYPE_PALETTE) {
        png_set_palette_to_rgb(png_ptr);
    }
    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
        png_set_tRNS_to_alpha(png_ptr);
        alpha = 1;
    }
    if (color_type == PNG_COLOR_TYPE_GRAY) {
        png_set_gray_to_rgb(png_ptr);
    }

    unsigned int y;
    if (channels == 3 || (channels == 1 && !alpha)) {
        for (y = 0; y < height; ++y) {
            unsigned char* pRow = pData + y * stride;
            png_read_row(png_ptr, pRow, NULL);

            int x;
            for(x = width - 1; x >= 0; x--) {
                int sx = x * 3;
                int dx = x * 4;
                unsigned char r = pRow[sx];
                unsigned char g = pRow[sx + 1];
                unsigned char b = pRow[sx + 2];
                unsigned char a = 0xff;
                pRow[dx    ] = r; // r
                pRow[dx + 1] = g; // g
                pRow[dx + 2] = b; // b
                pRow[dx + 3] = a;
            }
        }
    } else {
        for (y = 0; y < height; ++y) {
            unsigned char* pRow = pData + y * stride;
            png_read_row(png_ptr, pRow, NULL);
        }
    }

    *pSurface = surface;

exit:
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

    if (fp != NULL) {
        fclose(fp);
    }
    if (result < 0) {
        if (surface) {
            free(surface);
        }
    }
    return result;
}

void res_free_surface(GGLSurface* pSurface) {
    if (pSurface) {
        free(pSurface);
    }
}

int fb_init(struct fb_dev *dev)
{
    dev->fd = open("/dev/graphics/fb0", O_RDWR);
    if (dev->fd < 0) {
        dev->fd = open("/dev/fb0", O_RDWR);
        if (dev->fd < 0) {
            perror("open fb");
            return -1;
        }
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

    return 0;
}

void fb_exit(struct fb_dev *dev)
{
    munmap(dev->fbmem, dev->fblen);
    close(dev->fd);
}

#define PACKRGB16(r,g,b) (unsigned short) ((((b) & 0xF8) << 8 ) | (((g) & 0xFC) << 3 ) | (((r) & 0xF8) >> 3 ))

void to_rgb16(uint16_t *dst, uint8_t *src, int height, int width)
{
    int h, w;
    int r, g, b;

    for (h = 0; h < height; h++) {
        for (w = 0; w < width; w++) {
            int sx = h*width*4 + w*4;
            int dx = h*width*2 + w*2;

            b = src[sx+0];
            g = src[sx+1];
            r = src[sx+2];

            dst[dx/2] = PACKRGB16(r, g, b);
        }
    }
}

void to_rgb32(uint8_t *dst, uint8_t *src, int height, int width)
{
    int h, w;
    int r, g, b;

    for (h = 0; h < height; h++) {
        for (w = 0; w < width; w++) {
            int sx = h*width*4 + w*4;
            int dx = h*width*4 + w*4;

            b = src[sx+0];
            g = src[sx+1];
            r = src[sx+2];

            dst[dx+0] = r;
            dst[dx+1] = g;
            dst[dx+2] = b;
            dst[dx+3] = 0xff;
        }
    }
}

int fb_post(struct fb_dev *dev)
{
    uint32_t w = dev->var.xres;
    uint32_t h = dev->var.yres;

    if ((dev->pSurface->width != w) ||
            (dev->pSurface->height != h)) {
        return -1;
    }

    if (dev->var.bits_per_pixel == 16)
        to_rgb16((uint16_t *)dev->fbmem, dev->pSurface->data, h, w);
    else if (dev->var.bits_per_pixel == 32)
        to_rgb32(dev->fbmem, dev->pSurface->data, h, w);

    dev->var.yoffset = 0;
    ioctl(dev->fd, FBIOPUT_VSCREENINFO, &dev->var);
    ioctl(dev->fd, FBIOBLANK, FB_BLANK_UNBLANK);

    return 0;
}

int show_logo(int argc, char *argv[])
{
    struct fb_dev dev;
    char path[256];

    if (fb_init(&dev) < 0)
        return -1;

#if __arm__
    snprintf(path, sizeof(path), "/%dx%d.png", dev.var.xres, dev.var.yres);
#else
    snprintf(path, sizeof(path), "/tmp/%dx%d.png", dev.var.xres, dev.var.yres);
#endif

    if (res_create_surface(path, &dev.pSurface) < 0)
        return -1;

    if (fb_post(&dev) < 0)
        return -1;

    res_free_surface(dev.pSurface);

    /*
     * TODO:
     * rk-fb have to keep open, or it cannot display.
     * how to keep it opened ?
     */
    while (1)
        usleep(1000*1000*1000UL);

    fb_exit(&dev);

    return 0;
}

int main(int argc, char *argv[])
{
    int pid;

    pid = fork();

    if (pid == 0) {
        show_logo(0, 0);
    } else if (pid > 0) {
        ;
    }

    return 0;
}
