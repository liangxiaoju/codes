#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include "utils.h"

static int *LutYr = NULL;
static int *LutYg = NULL;;
static int *LutYb = NULL;;
static int *LutVr = NULL;;
static int *LutVrY = NULL;;
static int *LutUb = NULL;;
static int *LutUbY = NULL;;
static int *LutRv = NULL;
static int *LutGu = NULL;
static int *LutGv = NULL;
static int *LutBu = NULL;

#if 1
#define RGB24_TO_Y(r,g,b) LutYr[(r)] + LutYg[(g)] + LutYb[(b)]
#define YR_TO_V(r,y) LutVr[(r)] + LutVrY[(y)]
#define YB_TO_U(b,y) LutUb[(b)] + LutUbY[(y)]

#define R_FROMYV(y,v)  CLIP((y) + LutRv[(v)])
#define G_FROMYUV(y,u,v) CLIP((y) + LutGu[(u)] + LutGv[(v)])
#define B_FROMYU(y,u) CLIP((y) + LutBu[(u)])

#else

unsigned char
RGB24_TO_Y(unsigned char r, unsigned char g, unsigned char b)
{
return (LutYr[(r)] + LutYg[(g)] + LutYb[(b)]);
}
unsigned char
YR_TO_V(unsigned char r, unsigned char y)
{
return (LutVr[(r)] + LutVrY[(y)]);
}
unsigned char
YB_TO_U(unsigned char b, unsigned char y)
{
return (LutUb[(b)] + LutUbY[(y)]);
}
unsigned char
R_FROMYV(unsigned char y, unsigned char v)
{
return CLIP((y) + LutRv[(v)]);
}
unsigned char
G_FROMYUV(unsigned char y, unsigned char u, unsigned char v)
{
return CLIP((y) + LutGu[(u)] + LutGv[(v)]);
}
unsigned char
B_FROMYU(unsigned char y, unsigned char u)
{
return CLIP((y) + LutBu[(u)]);
}
#endif

void initLut(void)
{
	int i;
	#define Rcoef 299 
	#define Gcoef 587 
	#define Bcoef 114 
	#define Vrcoef 711 //656 //877 
	#define Ubcoef 560 //500 //493 564
	
	#define CoefRv 1402
	#define CoefGu 714 // 344
	#define CoefGv 344 // 714
	#define CoefBu 1772
	
	LutYr = malloc(256*sizeof(int));
	LutYg = malloc(256*sizeof(int));
	LutYb = malloc(256*sizeof(int));
	LutVr = malloc(256*sizeof(int));
	LutVrY = malloc(256*sizeof(int));
	LutUb = malloc(256*sizeof(int));
	LutUbY = malloc(256*sizeof(int));
	
	LutRv = malloc(256*sizeof(int));
	LutGu = malloc(256*sizeof(int));
	LutGv = malloc(256*sizeof(int));
	LutBu = malloc(256*sizeof(int));
	for (i= 0;i < 256;i++){
	    LutYr[i] = i*Rcoef/1000 ;
	    LutYg[i] = i*Gcoef/1000 ;
	    LutYb[i] = i*Bcoef/1000 ;
	    LutVr[i] = i*Vrcoef/1000;
	    LutUb[i] = i*Ubcoef/1000;
	    LutVrY[i] = 128 -(i*Vrcoef/1000);
	    LutUbY[i] = 128 -(i*Ubcoef/1000);
	    LutRv[i] = (i-128)*CoefRv/1000;
	    LutBu[i] = (i-128)*CoefBu/1000;
	    LutGu[i] = (128-i)*CoefGu/1000;
	    LutGv[i] = (128-i)*CoefGv/1000;
	}	
}


void freeLut(void){
	free(LutYr);
	free(LutYg);
	free(LutYb);
	free(LutVr);
	free(LutVrY);
	free(LutUb);
	free(LutUbY);
	
	free(LutRv);
	free(LutGu);
	free(LutGv);
	free(LutBu);
}

#define  FOUR_TWO_TWO 2		//Y00 Cb Y01 Cr
unsigned int
Pyuv422torgb24(unsigned char * input_ptr, unsigned char * output_ptr, unsigned int image_width, unsigned int image_height)
{
	unsigned int i, size;
	unsigned char Y, Y1, U, V;
	unsigned char *buff = input_ptr;
	unsigned char *output_pt = output_ptr;
	size = image_width * image_height /2;
	for (i = size; i > 0; i--) {
		/* bgr instead rgb ?? */
		Y = buff[0] ;
		U = buff[1] ;
		Y1 = buff[2];
		V = buff[3];
		buff += 4;
		*output_pt++ = B_FROMYU(Y,U); //v
		*output_pt++ = G_FROMYUV(Y,U,V); //b
		*output_pt++ = R_FROMYV(Y,V);
			
		*output_pt++ = B_FROMYU(Y1,U); //v
		*output_pt++ = G_FROMYUV(Y1,U,V); //b
		*output_pt++ = R_FROMYV(Y1,V);
	}
	
	return FOUR_TWO_TWO;
} 

unsigned int
Pyuv422torgb32(unsigned char * input_ptr, unsigned char * output_ptr, unsigned int image_width, unsigned int image_height)
{
	unsigned int i, size;
	unsigned char Y, Y1, U, V;
	unsigned char *buff = input_ptr;
	unsigned char *output_pt = output_ptr;
	size = image_width * image_height /2;
	for (i = size; i > 0; i--) {
		/* bgr instead rgb ?? */
		Y = buff[0] ;
		U = buff[1] ;
		Y1 = buff[2];
		V = buff[3];
		buff += 4;
		*output_pt++ = B_FROMYU(Y,U); //v
		*output_pt++ = G_FROMYUV(Y,U,V); //b
		*output_pt++ = R_FROMYV(Y,V);
        *output_pt++ = 0;
			
		*output_pt++ = B_FROMYU(Y1,U); //v
		*output_pt++ = G_FROMYUV(Y1,U,V); //b
		*output_pt++ = R_FROMYV(Y1,V);
        *output_pt++ = 0;
	}
	
	return FOUR_TWO_TWO;
} 

unsigned int
Pyuv422torgb16(unsigned char * input_ptr, unsigned char * output_ptr, unsigned int image_width, unsigned int image_height)
{
	unsigned int i, size;
	unsigned char Y, Y1, U, V;
	unsigned char *buff = input_ptr;
	unsigned short *output_pt = (unsigned short *)output_ptr;
	size = image_width * image_height /2;
	for (i = size; i > 0; i--) {
		/* bgr instead rgb ?? */
		Y = buff[0] ;
		U = buff[1] ;
		Y1 = buff[2];
		V = buff[3];
		buff += 4;
        *output_pt++ = PACKRGB16(
                B_FROMYU(Y,U), G_FROMYUV(Y,U,V), R_FROMYV(Y,V));

        *output_pt++ = PACKRGB16(
                B_FROMYU(Y1,U), G_FROMYUV(Y1,U,V), R_FROMYV(Y1,V));
	}

	return FOUR_TWO_TWO;
}

#if 0
int convert_yuv_to_rgb_pixel(int y, int u, int v)
{
    unsigned int pixel32 = 0;
    unsigned char *pixel = (unsigned char *)&pixel32;
    int r, g, b;
    r = y + (1.370705 * (v-128));
    g = y - (0.698001 * (v-128)) - (0.337633 * (u-128));
    b = y + (1.732446 * (u-128));
    if(r > 255) r = 255;
    if(g > 255) g = 255;
    if(b > 255) b = 255;
    if(r < 0) r = 0;
    if(g < 0) g = 0;
    if(b < 0) b = 0;
    pixel[0] = r ;
    pixel[1] = g ;
    pixel[2] = b ;
    return pixel32;
}

int convert_yuv_to_rgb_buffer(unsigned char *yuv, unsigned char *rgb, unsigned int width, unsigned int height)
{
    unsigned int in, out = 0;
    unsigned int pixel_16;
    unsigned char pixel_24[3];
    unsigned int pixel32;
    int y0, u, y1, v;

    for(in = 0; in < width * height * 2; in += 4)
    {
        pixel_16 =
            yuv[in + 3] << 24 |
            yuv[in + 2] << 16 |
            yuv[in + 1] <<  8 |
            yuv[in + 0];
        y0 = (pixel_16 & 0x000000ff);
        u  = (pixel_16 & 0x0000ff00) >>  8;
        y1 = (pixel_16 & 0x00ff0000) >> 16;
        v  = (pixel_16 & 0xff000000) >> 24;
        pixel32 = convert_yuv_to_rgb_pixel(y0, u, v);
        pixel_24[0] = (pixel32 & 0x000000ff);
        pixel_24[1] = (pixel32 & 0x0000ff00) >> 8;
        pixel_24[2] = (pixel32 & 0x00ff0000) >> 16;
        rgb[out++] = pixel_24[0];
        rgb[out++] = pixel_24[1];
        rgb[out++] = pixel_24[2];
        pixel32 = convert_yuv_to_rgb_pixel(y1, u, v);
        pixel_24[0] = (pixel32 & 0x000000ff);
        pixel_24[1] = (pixel32 & 0x0000ff00) >> 8;
        pixel_24[2] = (pixel32 & 0x00ff0000) >> 16;
        rgb[out++] = pixel_24[0];
        rgb[out++] = pixel_24[1];
        rgb[out++] = pixel_24[2];
    }
    return 0;

}
#endif
