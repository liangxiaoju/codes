/*
 * dump png image to stdout
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <png.h>


int main(int argc, const char *argv[])
{
	png_structp png_ptr;
	png_infop info_ptr;
	int height, pixel_size, i;
	int row_size;
	png_bytepp row_pointers;
	FILE *fp;

	fp = fopen(argv[1], "rb");
	if (!fp) {
		fprintf(stderr, "open %s error", argv[1]);
		return -1;
	}

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	info_ptr = png_create_info_struct(png_ptr);
	png_init_io(png_ptr, fp);

	
	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

	height = png_get_image_height(png_ptr, info_ptr);
	row_size = png_get_rowbytes(png_ptr, info_ptr);

	row_pointers = png_get_rows(png_ptr, info_ptr);


	/* dump raw image data */
	for (i = 0; i < height; i++)
		write(STDOUT_FILENO, row_pointers[i], row_size);

	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	fclose(fp);

	return 0;
}
