#include <common.h>
#include <command.h>
#include <regs.h>
#include <asm/io.h>
#include <linux/mtd/mtd.h>

typedef struct bmp_color_table_entry {
	__u8	blue;
	__u8	green;
	__u8	red;
	__u8	reserved;
} __attribute__ ((packed)) bmp_color_table_entry_t;

/* When accessing these fields, remember that they are stored in little
   endian format, so use linux macros, e.g. le32_to_cpu(width)          */

typedef struct bmp_header {
	/* Header */
	char signature[2];
	__u32	file_size;
	__u32	reserved;
	__u32	data_offset;
	/* InfoHeader */
	__u32	size;
	__u32	width;
	__u32	height;
	__u16	planes;
	__u16	bit_count;
	__u32	compression;
	__u32	image_size;
	__u32	x_pixels_per_m;
	__u32	y_pixels_per_m;
	__u32	colors_used;
	__u32	colors_important;
	/* ColorTable */

} __attribute__ ((packed)) bmp_header_t;

typedef struct bmp_image {
	bmp_header_t header;
	/* We use a zero sized array just as a placeholder for variable
	   sized array */
	bmp_color_table_entry_t color_table[0];
} bmp_image_t;

/* Data in the bmp_image is aligned to this length */
#define BMP_DATA_ALIGN	4

/* Constants for the compression field */
#define BMP_BI_RGB	0
#define BMP_BI_RLE8	1
#define BMP_BI_RLE4	2

#define Outp32(addr, data)	(*(volatile u32 *)(addr) = (data))
#define Inp32(_addr)		readl(_addr)

struct s3cfb_lcd_timing {
	int	h_fp;
	int	h_bp;
	int	h_sw;
	int	v_fp;
	int	v_fpe;
	int	v_bp;
	int	v_bpe;
	int	v_sw;
};
struct s3cfb_lcd_polarity {
	int	rise_vclk;
	int	inv_hsync;
	int	inv_vsync;
	int	inv_vden;
};
typedef struct {
	int	width;
	int	height;
	int	bpp;
	int	freq;
	struct	s3cfb_lcd_timing timing;
	struct	s3cfb_lcd_polarity polarity;
} s3cfb_lcd_t;

static s3cfb_lcd_t lcd_type_lg = {
	.width	= 1024,
	.height	= 768,
	.bpp	= 32,
	.freq	= 60,

	.timing = {
		.h_fp	= 10,
		.h_bp	= 6,
		.h_sw	= 2,
		.v_fp	= 6,
		.v_fpe	= 16,
		.v_bp	= 16,
		.v_bpe	= 16,
		.v_sw	= 10,
	},

	.polarity = {
		.rise_vclk	= 0,
		.inv_hsync	= 0,
		.inv_vsync	= 0,
		.inv_vden	= 0,
	},
};

static s3cfb_lcd_t *s3cfb_lcd;

/* TODO: distinguish lcds */
static void s3cfb_distinguish_lcd(void)
{
	/* here just choose the most common lcd type */
	s3cfb_lcd = &lcd_type_lg;
}
static void s3cfb_regulator_enable(void)
{
	/* poweron VFPGA_VCC_PWREN */
	writel((readl(GPA0CON) & 0x0fffffff) | 0x10000000, GPA0CON);
	writel(readl(GPA0PUD) & (~(3<<14)), GPA0PUD);
	writel(readl(GPA0DAT) | (1<<7), GPA0DAT);

	/* poweron VLCD_VCC_PWREN */
	writel((readl(GPG0CON) & 0xfffff0ff) | 0x00000100, GPG0CON);
	writel(readl(GPG0PUD) & (~(3<<4)), GPG0PUD);
	writel(readl(GPG0DAT) | (1<<2), GPG0DAT);

	/* set LCD_STB high */
	writel((readl(GPF3CON) & 0xfff0ffff) | 0x00010000, GPF3CON);
	writel(readl(GPF3PUD) & (~(3<<8)), GPF3PUD);
	writel(readl(GPF3DAT) | (1<<4), GPF3DAT);
}
static void s3cfb_cfg_gpio(void)
{
	Outp32(0xe0200120, 0x22222222);	//set GPF0 as LVD_HSYNC,VSYNC,VCLK,VDEN,VD[3:0]
	Outp32(0xe0200128,0x0);			//set pull-up,down disable
	Outp32(0xe0200140, 0x22222222);	//set GPF1 as VD[11:4]
	Outp32(0xe0200148,0x0);			//set pull-up,down disable
	Outp32(0xe0200160, 0x22222222);	//set GPF2 as VD[19:12]
	Outp32(0xe0200168,0x0);			//set pull-up,down disable
	Outp32(0xe0200180, 0x00002222);	//set GPF3 as VD[23:20]
	Outp32(0xe0200188,0x0);			//set pull-up,down disable

	/* min drive strength */
	Outp32(0xe020012c,0x0);
	Outp32(0xe020014c,0x0);
	Outp32(0xe020016c,0x0);
	Outp32(0xe020018c,0x0);
}
static void s3cfb_clk_enable(void)
{
	/* syscon output path */
	Outp32(0xe0107008,0x2);
	/* syscon fimdclk = mpll */
	Outp32(0xe0100204, (Inp32(0xe0100204) & (~(0xf<<20))) | 6<<20);
	/* SCLK_FIMD=MOUTFIMD/(FIMD_RATIO+1): (166750000) */
	Outp32(0xe0100304, (Inp32(0xe0100304) & (~(0xf<<20))) | 3<<20);
	/* enable MUXFIMD */
	Outp32(0xe0100280, Inp32(0xe0100280) | 1<<5);
	/* CLK_FIMD pass */
	Outp32(0xe0100464, Inp32(0xe0100464) | (1<<0));
	/* CLK_LCD pass */
	Outp32(0xe0100480, Inp32(0xe0100480) | (1<<3));
}
static void s3cfb_set_output(void)
{
	u32 cfg;

	/* RGB */
	cfg = readl(0xf8000000);
	cfg &= ~(7<<26);
	cfg |= 0<<26;
	writel(cfg, 0xf8000000);

	cfg = readl(0xf8000008);
	cfg &= ~((1<<15) | (1<<14) | (3<<12));
	cfg |= 0<<15;
	writel(cfg, 0xf8000008);
}
static void s3cfb_set_display_mode(void)
{
	u32 cfg;

	/* RGB_P */
	cfg = readl(0xf8000000);
	cfg &= ~(3<<17);
	cfg |= 0<<17;
	writel(cfg, 0xf8000000);
}
static void s3cfb_set_polarity(void)
{
	u32 cfg = 0;
	struct s3cfb_lcd_polarity *pol = &s3cfb_lcd->polarity;

	if (pol->rise_vclk)
		cfg |= 1<<7;
	if (pol->inv_hsync)
		cfg |= 1<<6;
	if (pol->inv_vsync)
		cfg |= 1<<5;
	if (pol->inv_vden)
		cfg |= 1<<4;

	writel(cfg, 0xf8000004);
}
static void s3cfb_set_timing(void)
{
	u32 cfg = 0;
	struct s3cfb_lcd_timing *time = &s3cfb_lcd->timing;

	cfg |= (time->v_bpe-1)<<24;
	cfg |= (time->v_bp-1)<<16;
	cfg |= (time->v_fp-1)<<8;
	cfg |= (time->v_sw-1)<<0;
	writel(cfg, 0xf8000010);

	cfg = 0;
	cfg |= (time->v_fpe-1)<<24;
	cfg |= (time->h_bp-1)<<16;
	cfg |= (time->h_fp-1)<<8;
	cfg |= (time->h_sw-1)<<0;
	writel(cfg, 0xf8000014);
}
static void s3cfb_set_lcd_size(void)
{
	u32 cfg = 0;

	cfg |= (s3cfb_lcd->width-1)<<0;
	cfg |= (s3cfb_lcd->height-1)<<11;
	writel(cfg, 0xf8000018);
}
/* TODO: */
static void s3cfb_set_window_control(void)
{
	u32 cfg;

	cfg = readl(0xf8000020);
	cfg &= ~((1<<18) | (1<<17) | (1<<16) | (1<<15) | (3<<9) | (0xf<<2) | (1<<13) | (1<<22));
	/* dma */
	cfg |= 0<<22;
	/* bpp=32 */
	cfg |= 1<<15;
	/* dma_burst=16 */
	cfg |= 0<<9;
	/* A8-R8-G8-B8 */
	cfg |= 0xd<<2;
	writel(cfg, 0xf8000020);
}
static void s3cfb_set_window_position(void)
{
	u32 cfg, shw;
/*
	shw = readl(0xf8000034);
	shw |= 0<<10;
	writel(shw, 0xf8000034);
*/
	cfg = (0<<11) | (0<<0);
	writel(cfg, 0xf8000040);

	cfg = ((0+s3cfb_lcd->width-1)<<11) | (0+s3cfb_lcd->height-1);
	writel(cfg, 0xf8000044);

	/* TODO: */
	/*
	shw = readl(0xf8000034);
	shw &= ~(0<<10);
	writel(shw, 0xf8000034);
	*/
}
static void s3cfb_set_window_size(void)
{
	u32 cfg;

	cfg = s3cfb_lcd->width * s3cfb_lcd->height;
	writel(cfg, 0xf8000048);
}
static void s3cfb_set_buffer_address(uint32_t fbaddr)
{
	u32 start_addr, end_addr;

	start_addr = fbaddr;
	end_addr = start_addr + s3cfb_lcd->width*s3cfb_lcd->height*s3cfb_lcd->bpp/8;
	writel(start_addr, 0xf80000a0);
	writel(end_addr, 0xf80000d0);
}
static void s3cfb_set_buffer_size(void)
{
	u32 cfg, offset, pw;

	pw = s3cfb_lcd->width*s3cfb_lcd->bpp/8;
	offset = 0<<13;
	cfg = offset | pw;
	writel(cfg, 0xf8000100);
}
static void s3cfb_set_win_params(uint32_t fbaddr)
{
	s3cfb_set_window_control();
	s3cfb_set_window_position();
	s3cfb_set_window_size();
	s3cfb_set_buffer_address(fbaddr);
	s3cfb_set_buffer_size();
}
static void s3cfb_set_clock(void)
{
	u32 cfg, div;
	u32 pixclock;
	u32 src_clk = 166750000;
	struct s3cfb_lcd_timing *timing = &s3cfb_lcd->timing;

	pixclock = s3cfb_lcd->freq *
		(timing->h_fp+timing->h_bp+timing->h_sw+s3cfb_lcd->width) *
		(timing->v_fp+timing->v_bp+timing->v_sw+s3cfb_lcd->height);

	cfg = readl(0xf8000000);
	cfg &= ~((1<<2) | (1<<16) | (1<<5) | (1<<4));
	cfg |= (0<<16)|(0<<5)|(1<<4);
	/* video clock source: sclk_fimd */
	cfg |= 1<<2;
	/* cacl div */
	div = src_clk / pixclock;
	if (src_clk % pixclock)
		div ++;
	cfg |= (div-1)<<6;
	writel(cfg, 0xf8000000);
}
static void s3cfb_window_on(void)
{
	u32 cfg;

	cfg = readl(0xf8000020);
	cfg |= 1<<0;
	writel(cfg, 0xf8000020);

	writel(0x1, 0xf8000034);
}
static void s3cfb_display_on(void)
{
	u32 cfg;

	cfg = readl(0xf8000000);
	cfg |= (1<<0) | (1<<1);
	writel(cfg, 0xf8000000);
}
static void s3cfb_init(uint32_t fbaddr)
{
	s3cfb_cfg_gpio();
	s3cfb_clk_enable();
	s3cfb_regulator_enable();
	s3cfb_distinguish_lcd();
	s3cfb_set_output();
	s3cfb_set_display_mode();
	s3cfb_set_polarity();
	s3cfb_set_timing();
	s3cfb_set_lcd_size();
	s3cfb_set_win_params(fbaddr);
	s3cfb_set_clock();
	s3cfb_window_on();
	s3cfb_display_on();
}

static int do_show_logo(uint32_t fbaddr)
{
	s3cfb_init(fbaddr);

	return 0;
}

/* TODO: load logo by partition name, not address */
static uint32_t get_logo_addr(int index)
{
	uint32_t const logo1addr = 0x00e00000;
	uint32_t const logo2addr = 0x00e00000 + 5*1024*1024;

	switch (index) {
		case 1:
			return logo1addr;
		case 2:
			return logo2addr;
		default:
			return 0;
	}
}

static int load_bmp(uint32_t dst, uint32_t src, uint32_t size)
{
	char cmd[64];

	sprintf(cmd, "onenand read 0x%08x 0x%08x 0x%08x", dst, src, size);
	run_command(cmd, 0);

	return 0;
}

static int bmptorgb(uint32_t fbaddr, uint32_t bmpaddr)
{
	bmp_header_t *header;
	char sig[2];
	uint32_t width, height, data_offset, image_size, file_size;
	uint32_t line_len;
	int i;
	char *src, *dst;

	header = (bmp_header_t *)bmpaddr;

	sig[0] = header->signature[0];
	sig[1] = header->signature[1];
	if (!((sig[0] == 'B') && (sig[1] == 'M')))
		return -1;

	width = le32_to_cpu(header->width);
	height = le32_to_cpu(header->height);
	data_offset = le32_to_cpu(header->data_offset);
	image_size = le32_to_cpu(header->image_size);
	file_size = le32_to_cpu(header->file_size);
	dst = (char *)fbaddr;
	src = (char *)(bmpaddr + data_offset);
	line_len = image_size / height;
	for (i = height-1; i >= 0; i--) {
		memcpy((dst+(i*line_len)), src, line_len);
		src += line_len;
	}

	printf("logo[0x%08x@0x%08x(%c%c)]: %dx%d %d@0x%x\n",
			file_size, bmpaddr, sig[0], sig[1],
			width, height, image_size, data_offset);

	return 0;
}

static int video_setenv(uint32_t addr1, uint32_t addr2)
{
	char env[64];
	char logo2[32];

	if ((addr1 != 0) && (addr2 != 0))
		sprintf(env, "s3cfb:logo1addr:0x%08x,logo2addr:0x%08x", addr1, addr2);
	else if (addr1 != 0)
		sprintf(env, "s3cfb:logo1addr:0x%08x", addr1);
	else if (addr2 != 0)
		sprintf(env, "s3cfb:logo2addr:0x%08x", addr2);
	else
		env[0] = 0;

	if (env[0])
		setenv("video", env);

	return 0;
}

static int do_load_logo(uint32_t fbaddr)
{
	int ret;
	int ok1 = 0, ok2 = 0;
	uint32_t logo1addr, logo2addr;
	uint32_t bmpaddr = fbaddr + 10*1024*1024;
	uint32_t bmpsize = 4*1024*1024;

	logo1addr = get_logo_addr(1);
	logo2addr = get_logo_addr(2);

	load_bmp(bmpaddr, logo1addr, bmpsize);
	ret = bmptorgb(fbaddr, bmpaddr);
	if (ret == 0)
		ok1 = 1;

	load_bmp(bmpaddr, logo2addr, bmpsize);
	ret = bmptorgb(fbaddr + 5*1024*1024, bmpaddr);
	if (ret == 0)
		ok2 = 1;

	video_setenv(fbaddr*ok1, (fbaddr+5*1024*1024)*ok2);

	return 0;
}

int do_logo(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	uint32_t fbaddr;

	fbaddr = simple_strtoul(argv[2], NULL, 0);

	if (fbaddr == 0)
		fbaddr = CFG_LCD_FBUFFER;

	if (!strcmp(argv[1], "show")) {

		do_show_logo(fbaddr);

	} else if (!strcmp(argv[1], "load")) {

		do_load_logo(fbaddr);

	} else {
		printf("Usage:\n%s\n", cmdtp->usage);
		return -1;
	}

	return 0;
}

U_BOOT_CMD(
	logo, 3, 0, do_logo,
	"logo	- boot logo.\n",
	"show <addr>		- show logo.\n"
	"logo load <addr>	- load logo.\n"
);
