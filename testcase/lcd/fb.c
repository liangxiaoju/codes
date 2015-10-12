#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/fb.h>
#include <string.h>
#include <png.h>

struct ebc_buf_info{
	int offset;
	int epd_mode;
	int height;
	int width;
	int vir_height;
	int vir_width;
};

struct ebc_dev {
        int ebc_fd;
        int rotate;
        struct fb_var_screeninfo vi;
        int row;
        int col;
        int buflen;
        void *bufaddr;
        struct ebc_buf_info info;
};

#define GET_EBC_BUFFER 0x7000
#define SET_EBC_SEND_BUFFER 0x7001
#define GET_EBC_BUFFER_INFO 0x7003

#define EBC_DEV "/dev/ebc"

static struct ebc_dev ebc_dev;

int ebc_init(struct ebc_dev *dev)
{
        struct ebc_buf_info *info = &dev->info;

        dev->ebc_fd = open(EBC_DEV, O_RDWR);
        if (dev->ebc_fd < 0) {
                perror("open");
                return -1;
        }

        if (ioctl(dev->ebc_fd, GET_EBC_BUFFER_INFO, info) != 0) {
                perror("ioctl");
                return -1;
        }

        dev->buflen = info->vir_width * info->vir_height * 2;
        dev->bufaddr = mmap(0, dev->buflen, PROT_READ | PROT_WRITE, MAP_SHARED, dev->ebc_fd, 0);
        if (dev->bufaddr == MAP_FAILED) {
                perror("mmap");
                return -1;
        }

        printf("ebc_init: vwidth=%d vheight=%d offset=%d mode=%d\n",
               info->vir_width, info->vir_height, info->offset,
               info->epd_mode);

        return 0;
}

int ebc_test(struct ebc_dev *dev)
{
        struct ebc_buf_info *info = &dev->info;

        if (ioctl(dev->ebc_fd, GET_EBC_BUFFER, info) != 0) {
                perror("ioctl GET_EBC_BUFFER");
                return -1;
        }

        printf("ebc_test: w=%d h%d offset=%d mode=%d\n", info->vir_width, info->vir_height, info->offset, info->epd_mode);
        info->epd_mode = 1;
        memset(dev->bufaddr + info->offset, 0x00, dev->buflen/4);

        if (ioctl(dev->ebc_fd, SET_EBC_SEND_BUFFER, info) != 0) {
                perror("ioctl SET_EBC_SEND_BUFFER");
                return -1;
        }
        printf("ebc_test: mode=%d\n", info->epd_mode);

        return 0;
}

#define RGB2Luma_4bit(r,g,b)        ((77*r+150*g+29*b)>>12)
void RGB888_2_Luma_0(unsigned short *luma, unsigned char *src, unsigned int width)
{
	unsigned int i;
	unsigned int r, g, b, g0, g1, g2, g3;

	for (i = 0; i < width; i += 4) {
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

		*luma++ = (g0 & 0x0F) | ((g1 & 0xF)<<4) | ((g2 & 0xF)<<8)| ((g3 & 0xF)<<12);
	}
}

int ebc_show_png(struct ebc_dev *dev, const char *file)
{
        struct ebc_buf_info *info = &dev->info;
        png_structp png_ptr;
	png_infop info_ptr;
	int height, pixel_size, i;
	int row_size;
	png_bytepp row_pointers;
	FILE *fp;

	fp = fopen(file, "rb");
	if (!fp) {
		fprintf(stderr, "open %s error", file);
		return -1;
	}

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	info_ptr = png_create_info_struct(png_ptr);
	png_init_io(png_ptr, fp);

	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

	height = png_get_image_height(png_ptr, info_ptr);
	row_size = png_get_rowbytes(png_ptr, info_ptr);

	row_pointers = png_get_rows(png_ptr, info_ptr);

        if (ioctl(dev->ebc_fd, GET_EBC_BUFFER, info) != 0) {
                perror("ioctl GET_EBC_BUFFER");
                return -1;
        }
        info->epd_mode = 1;

	/* dump raw image data */
	for (i = 0; i < height; i++) {
                RGB888_2_Luma_0(
                        dev->bufaddr + info->offset + (info->width * 2 / 4)*i,
                        row_pointers[i], info->vir_width);
        }

        if (ioctl(dev->ebc_fd, SET_EBC_SEND_BUFFER, info) != 0) {
                perror("ioctl SET_EBC_SEND_BUFFER");
                return -1;
        }

	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	fclose(fp);
}

int main(int argc, char *argv[])
{
        struct ebc_dev *dev = &ebc_dev;
        struct ebc_buf_info *info = &dev->info;
        int ret;

        if (ebc_init(dev)) {
                perror("ebc_init");
                return -1;
        }

//        ebc_test(dev);
        ebc_show_png(dev, argv[1]);

        return 0;
}
