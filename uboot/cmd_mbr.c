#include <common.h>
#include <command.h>
#include <malloc.h>
#include <mmc.h>
#include <regs.h>
#include <asm/io.h>

#define PART_PRIMARY		1
#define PART_EXTENDED		2
#define PART_LOGICAL		3

#define LINUX_NATIVE		0x83
#define EXTENDED		0x05
#define SIZE_REMAINING		0xFFFFFFFF
#define OFFSET_NOT_SPECIFIED	0xFFFFFFFF

/* offset between last partition and next MBR */
#define MBR_OFFSET		0
#define SECTOR_SIZE		512
#define BLOCK_SIZE		512
#define MIN_PART_SIZE		4096
/* offset between MBR and partition */
#define PART_OFFSET		2048

#define DEFAULT_PART_ENV	"1G(iNand)p"

#define byte_to_sect(x)		(x / SECTOR_SIZE)
#define HEADS_PER_DISK		255
#define SECTORS_PER_TRACK	63

//#define USE_HSC
//#define MBR_DEBUG

#ifdef MBR_DEBUG
#define mbr_debug(fmt, arg...)	printf(fmt, ## arg)
#else
#define mbr_debug(fmt, arg...)
#endif


typedef struct {
	uint8_t boot_ind;         /* 0x80 - active */
	uint8_t start_head;       /* starting head */
	uint8_t start_sector;     /* starting sector */
	uint8_t start_cyl;        /* starting cylinder */
	uint8_t sys_ind;          /* What partition type */
	uint8_t end_head;         /* end head */
	uint8_t end_sector;       /* end sector */
	uint8_t end_cyl;          /* end cylinder */
	uint8_t start4[4];        /* starting sector counting from 0 */
	uint8_t size4[4];         /* nr of sectors in partition */
} dpt_t;

typedef struct {
	struct {
		int8_t bcode[446];
		dpt_t dpt[4];
		int8_t flags[2];
	}  __attribute__ ((__packed__));
	char *name;
} mbr_t;

typedef struct EBR {
	struct {
		int8_t bcode[446];
		dpt_t dpt[4];
		int8_t flags[2];
	}  __attribute__ ((__packed__));
	char *name;
	uint32_t offset_sect;
	struct EBR *prev;
	struct EBR *next;
} ebr_t;

typedef struct {
	mbr_t *mbr;
	ebr_t *ebr;
	uint32_t extended_offset_sect;
	uint32_t total_block_count;
} disk_t;


static void *zalloc(size_t bytes)
{
	void *p;
	p = malloc(bytes);
	if (!p)
		return NULL;
	memset(p, 0, bytes);
	return p;
}

static void store4_little_endian(uint8_t *p, uint32_t val)
{
	p[0] = val & 0xff;
	p[1] = (val >> 8) & 0xff;
	p[2] = (val >> 16) & 0xff;
	p[3] = (val >> 24) & 0xff;
}

static uint32_t read4_little_endian(uint8_t *p)
{
	return ((uint32_t)(p[0])) + ((uint32_t)(p[1])<<8)
		+ ((uint32_t)(p[2])<<16)
		+ ((uint32_t)(p[3])<<24);
}

static void set_start_sect(dpt_t *dpt, uint32_t start_sect)
{
	store4_little_endian(dpt->start4, start_sect);
}

static void set_nr_sects(dpt_t *dpt, uint32_t size_sect)
{
	store4_little_endian(dpt->size4, size_sect);
}

static uint32_t get_start_sect(dpt_t *dpt)
{
	return read4_little_endian(dpt->start4);
}

static uint32_t get_nr_sects(dpt_t *dpt)
{
	return read4_little_endian(dpt->size4);
}

#ifdef USE_HSC
static void lba2hsc(uint32_t lba, uint32_t *h, uint32_t *s, uint32_t *c)
{
	uint32_t spc = HEADS_PER_DISK * SECTORS_PER_TRACK;

	*c = lba / spc;
	lba = lba % spc;
	*h = lba / SECTORS_PER_TRACK;
	*s = lba % SECTORS_PER_TRACK + 1;	/* sectors count from 1 */
}

static uint32_t hsc2lba(uint32_t h, uint32_t s, uint32_t c)
{
	return (c * HEADS_PER_DISK + h) * SECTORS_PER_TRACK + (s - 1);
}

static void lba2packhsc(uint32_t lba, uint8_t *h, uint8_t *s, uint8_t *c)
{
	uint32_t head, sector, cylinder;

	lba2hsc(lba, &head, &sector, &cylinder);

	*h = head & 0xff;
	*s |= ((cylinder >> 2) & 0xc0);
	*c = cylinder & 0xff;
}

static uint32_t packhsc2lba(uint8_t *h, uint8_t *s, uint8_t *c)
{
	uint32_t head, sector, cylinder;

	head = *h;
	sector = *s & 0x3f;
	cylinder = *c | ((*s & 0xc0) << 2);
	return hsc2lba(head, sector, cylinder);
}

static void set_hsc(uint32_t sects, uint8_t *h, uint8_t *s, uint8_t *c)
{
	lba2packhsc(sects, h, s, c);
}
#endif

static void set_dpt(dpt_t *dpt, uint32_t start_sect, uint32_t size_sect, uint8_t fs_id)
{
	dpt->boot_ind = 0;
	dpt->sys_ind = fs_id;

#ifdef USE_HSC
	set_hsc(start_sect, &dpt->start_head, &dpt->start_sector, &dpt->start_cyl);
	set_hsc(start_sect + size_sect, &dpt->end_head, &dpt->end_sector, &dpt->end_cyl);
#else
	dpt->start_head = 1;
	dpt->start_sector = 1;
	dpt->start_cyl = 0;
	dpt->end_head = 254;
	dpt->end_sector = 255;
	dpt->end_cyl = 254;
#endif

	set_start_sect(dpt, start_sect);
	set_nr_sects(dpt, size_sect);
	mbr_debug("%s: start_sect=%d size_sect=%d fs_id=0x%02x\n", __func__, start_sect, size_sect, fs_id);
}

static void write_part_table_flag(uint8_t *b)
{
	b[510] = 0x55;
	b[511] = 0xaa;
}
/*
static uint16_t read_part_table_flag(uint8_t *b)
{
	return (b[510] << 8) | b[511];
}
*/
static int pick_nonexisting_part(mbr_t *mbr, uint8_t flag)
{
	int i, n = -1;

	for (i = 0; i < 4; i++) {
		if ((mbr->dpt[i].sys_ind != LINUX_NATIVE) &&
			(mbr->dpt[i].sys_ind != EXTENDED)) {
			n = i;
			break;
		}
	}

	return n;
}

static int pick_last_part(mbr_t *mbr, uint8_t flag)
{
	int i, n = -1;

	for (i = 0; i < 4; i++) {
		if (mbr->dpt[i].sys_ind == flag)
			n = i;
	}

	return n;
}

static int mbr_add_primary(mbr_t *mbr, uint32_t offset_sect, uint32_t size_sect, char *name)
{
	int n;

	n = pick_nonexisting_part(mbr, LINUX_NATIVE);
	if (n < 0)
		return -1;
	mbr_debug("%s n=%d\n", __func__, n);

	set_dpt(&mbr->dpt[n], offset_sect, size_sect, LINUX_NATIVE);

	write_part_table_flag((uint8_t *)mbr);

	return 0;
}

static int mbr_add_extended(mbr_t *mbr, uint32_t offset_sect, uint32_t size_sect, char *name)
{
	int n;

	n = pick_nonexisting_part(mbr, EXTENDED);
	if (n < 0)
		return -1;
	mbr_debug("%s n=%d\n", __func__, n);

	set_dpt(&mbr->dpt[n], offset_sect, size_sect, EXTENDED);

	write_part_table_flag((uint8_t *)mbr);

	return 0;
}

static int ebr_add_logical(ebr_t *ebr, uint32_t offset_sect, uint32_t size_sect, char *name)
{
	uint32_t ebr_offset_sect;
	uint32_t extended_offset_sect = ebr->offset_sect;
	ebr_t *prev = ebr->prev;

	if (prev) {
		ebr_offset_sect = (prev->offset_sect - extended_offset_sect) + get_start_sect(&prev->dpt[0]) + get_nr_sects(&prev->dpt[0]);
		set_dpt(&prev->dpt[1], ebr_offset_sect, ebr_offset_sect, EXTENDED);
		ebr->offset_sect += ebr_offset_sect;
	}
	set_dpt(&ebr->dpt[0], offset_sect, size_sect, LINUX_NATIVE);

	write_part_table_flag((uint8_t *)ebr);

	return 0;
}

static unsigned long memsize_parse (const char *const ptr, const char **retptr)
{
	unsigned long ret = simple_strtoul(ptr, (char **)retptr, 0);

	switch (**retptr) {
	case 'G':
	case 'g':
		ret <<= 10;
	case 'M':
	case 'm':
		ret <<= 10;
	case 'K':
	case 'k':
		(*retptr)++;
		break;
	default:
		ret = -1;
		break;
	}

	return ret;
}

static int adjust_offset_size(disk_t *disk, uint32_t *offset_sect, uint32_t *size_sect, uint32_t flag)
{
	int n=-1;
	uint32_t last_start_sect=0, last_size_sect=0;

	if (*offset_sect == OFFSET_NOT_SPECIFIED) {
		if ((flag == PART_PRIMARY) || (flag == PART_EXTENDED)) {
			n = pick_last_part(disk->mbr, LINUX_NATIVE);
			if (n < 0)
				*offset_sect = 0 + PART_OFFSET;
			else {
				last_start_sect = get_start_sect(&disk->mbr->dpt[n]);
				last_size_sect = get_nr_sects(&disk->mbr->dpt[n]);
				*offset_sect = last_start_sect + last_size_sect;
			}
		} else if (flag == PART_LOGICAL) {
			if (disk->ebr == NULL) {
				n = pick_last_part(disk->mbr, EXTENDED);
				if (n < 0)
					return -1;
				*offset_sect = 0 + PART_OFFSET;
			} else {
				ebr_t *ebr;
				for (ebr= disk->ebr; ebr->next != NULL; ebr = ebr->next)
					;
				last_start_sect = get_start_sect(&ebr->dpt[0]);
				last_size_sect = get_nr_sects(&ebr->dpt[0]);
				*offset_sect = PART_OFFSET;
			}
		}
	}

	if (*size_sect == SIZE_REMAINING) {
		uint32_t total_bc;
		total_bc = disk->total_block_count -10*1024*1024/SECTOR_SIZE;
		if ((flag == PART_PRIMARY) || (flag == PART_EXTENDED)) {
			*size_sect = total_bc - *offset_sect;
		} else if (flag == PART_LOGICAL) {
			if (disk->ebr == NULL) {
				n = pick_last_part(disk->mbr, EXTENDED);
				if (n < 0)
					return -1;
				*size_sect = total_bc - PART_OFFSET - disk->extended_offset_sect;
			} else {
				ebr_t *ebr;
				for (ebr= disk->ebr; ebr->next != NULL; ebr = ebr->next)
					;
				*size_sect =total_bc - ebr->offset_sect - get_start_sect(&ebr->dpt[0]) - get_nr_sects(&ebr->dpt[0]) - PART_OFFSET;
			}
		}
		mbr_debug("total_block_count=%d\n", disk->total_block_count);
	}
	mbr_debug("%s: last_part=%d last_start=%d last_size=%d flag=%d\n", __func__, n, last_start_sect, last_size_sect, flag);
	mbr_debug("%s: offset=%d size=%d\n", __func__, *offset_sect, *size_sect);

	return 0;
}

static int is_extended_exist(disk_t *disk)
{
	int i, n = 0;

	for (i = 0; i < 4; i++) {
		if (disk->mbr->dpt[i].sys_ind == EXTENDED)
			n++;
	}

	return n;
}

static int add_partition(disk_t *disk, uint32_t offset_sect, uint32_t size_sect, char *name, uint32_t flag)
{
	mbr_debug("** name=%s offset=%d size=%d, flag=%d\n", name, offset_sect, size_sect, flag);

	adjust_offset_size(disk, &offset_sect, &size_sect, flag);

	if (flag == PART_PRIMARY) {
		if (disk->mbr == NULL) {
			disk->mbr = zalloc(sizeof(mbr_t));
			if (!disk->mbr)
				return -1;
		}
		mbr_add_primary(disk->mbr, offset_sect, size_sect, name);

	} else if (flag == PART_EXTENDED) {
		if (disk->mbr == NULL)
			return -1;
		mbr_add_extended(disk->mbr, offset_sect, size_sect, name);
		disk->extended_offset_sect = offset_sect;

	} else if (flag == PART_LOGICAL) {
		ebr_t *ebr;
		if (!is_extended_exist(disk))
			return -1;
		if (disk->ebr == NULL) {
			disk->ebr = zalloc(sizeof(ebr_t));
			if (!disk->ebr)
				return -1;
			ebr = disk->ebr;
		} else {
			for (ebr = disk->ebr; ebr->next != NULL; ebr = ebr->next)
				;
			ebr->next = zalloc(sizeof(ebr_t));
			if (!ebr->next)
				return -1;
			ebr->next->prev = ebr;
			ebr = ebr->next;
		}
		ebr->offset_sect = disk->extended_offset_sect;
		ebr_add_logical(ebr, offset_sect, size_sect, name);
	}
	mbr_debug("\n");
	return 0;
}

static int parse_part(disk_t *disk, const char *p, const char **retp)
{
	char name[16] = {'\0'};
	uint32_t flag;
	uint64_t size;
	uint32_t size_sect, offset_sect;

	/* size */
	if (*p == '-') {
		size_sect = SIZE_REMAINING;
		p++;
	} else {
		size = memsize_parse(p, &p);
		if (size < MIN_PART_SIZE) {
			return -1;
		}
		size_sect = size*2;
	}

	/* offset */
	offset_sect = OFFSET_NOT_SPECIFIED;

	/* name */
	if (*p == '(') {
		const char *n = ++p;
		int l;
		if ((p = strchr(n, ')')) == NULL) {
			return -1;
		}
		l = p - n;
		if (l >= 0) {
			memcpy(name, n, l);
			name[l] = '\0';
		}
		p++;
	}

	/* p , l or e */
	if (*p == 'e') {
		flag = PART_EXTENDED;
		p++;
	} else if (*p == 'l') {
		flag = PART_LOGICAL;
		p++;
	} else {
		flag = PART_PRIMARY;
		p++;
	}

	if (*p == ',') {
		if (flag != PART_EXTENDED && size == SIZE_REMAINING) {
			return -1;
		}
		*retp = ++p;
	} else if (*p == '\0') {
		*retp = p;
	} else {
		return -1;
	}

	add_partition(disk, offset_sect, size_sect, name, flag);
	return 0;
}

static int parse_disk(disk_t *disk)
{
	const char *env;

	env = getenv("mbr");
	if (!env)
		env = DEFAULT_PART_ENV;

	while (env && (*env != '\0')) {
		if (parse_part(disk, env, &env) < 0)
			return -1;
	}

	return 0;
}

static struct mmc *get_mmc_device(int dev_num)
{
	int rv;
	struct mmc *mmc;

	mmc = find_mmc_device(dev_num);
	if (!mmc) {
		printf("mmc/sd device is NOT founded.\n");
		return NULL;
	}	

	rv = mmc_init(mmc);
	if (rv) {
		printf("mmc/sd device's initialization is failed.\n");
		return NULL;
	}

	return mmc;
}

static int get_mmc_block_count(int dev_num)
{
	int block_count = 0;
	struct mmc *mmc;

	mmc = get_mmc_device(dev_num);
	if (mmc == NULL)
		return -1;

	block_count = mmc->capacity * (mmc->read_bl_len / BLOCK_SIZE);

	return block_count;
}

#define MAX_PER_LINE	16
static void print_buf(uint8_t *buf, int dev_num)
{
	unsigned int i, l;

	for (i = 0, l = 0; i < SECTOR_SIZE; i++, l++) {
		if (l == 0)
			printf("0x%03X:", i);
		printf(" %02X", buf[i]);
		if (l == MAX_PER_LINE - 1) {
			printf("\n");
			l = -1;
		}
	}
	if (l > 0)
		printf("\n");
	printf("\n");
}

static int write_table(disk_t *disk, int dev_num)
{
	int rv;
	struct mmc *mmc;
	ebr_t *ebr;

	mmc = get_mmc_device(dev_num);
	if (mmc == NULL)
		return -1;

	rv = mmc->block_dev.block_write(dev_num, 0, 1, (uint8_t *)disk->mbr);
	if (rv != 1)
		return -1;
	mbr_debug("offset_sect=0\n");
#ifdef MBR_DEBUG
	print_buf((uint8_t *)disk->mbr, -1);
#endif

	for (ebr = disk->ebr; ebr != NULL; ebr = ebr->next) {
		if (!ebr->prev)
			ebr->offset_sect = disk->extended_offset_sect;
		rv = mmc->block_dev.block_write(dev_num, ebr->offset_sect, 1, (uint8_t *)ebr);
		if (rv != 1)
			return -1;
		mbr_debug("offset_sect=0x%x(%d)\n", ebr->offset_sect, ebr->offset_sect);
#ifdef MBR_DEBUG
		print_buf((uint8_t *)ebr, -1);
#endif
	}
	return 0;
}

static int new_partitions(disk_t *disk, int dev_num)
{
	parse_disk(disk);
	write_table(disk, dev_num);

	return 0;
}

static int list_table(disk_t *disk, int dev_num)
{
	int rv;
	struct mmc *mmc;
	char mbr[512] = {0};

	mmc = get_mmc_device(dev_num);
	if (mmc == NULL)
		return -1;

	rv = mmc->block_dev.block_read(dev_num, 0, 1, mbr);
	if (rv != 1)
		return -1;
	print_buf((uint8_t *)mbr, -1);

	return 0;
}

static void powerup_inand(void)
{
	/* GPJ4[2], enable ViNand */
	writel((readl(GPJ4CON) & 0xfffff0ff) | 0x00000100, GPJ4CON);
	writel(readl(GPJ4PUD) & (~(0x3<<4)), GPJ4PUD);
	writel(readl(GPJ4DAT) | (0x1<<2), GPJ4DAT);
	udelay(10);
}

static void powerup_sdcard(void)
{
	/* GPJ4[0], enable sdcard */
	writel((readl(GPJ4CON) & 0xfffffff0) | 0x00000001, GPJ4CON);
	writel(readl(GPJ4PUD) & (~(0x3<<0)), GPJ4PUD);
	writel(readl(GPJ4DAT) | (0x1<<0), GPJ4DAT);
	udelay(10);
}

static int get_dev_num(char *name)
{
	if (!strcmp(name, "inand")) {
		powerup_inand();
		return 0;
	} else if (!strcmp(name, "sdcard")) {
		powerup_sdcard();
		return 1;
	}
	return -1;
}

static void usage(void)
{
	printf("Usage: mbr <create or print> <inand or sdcard>\n");
}

static int do_mbr(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int dev_num;
	disk_t *disk;

	if (argc != 3) {
		usage();
		return -1;
	}

	disk = zalloc(sizeof(disk_t));
	if (!disk)
		return -1;

	dev_num = get_dev_num(argv[2]);
	disk->total_block_count = get_mmc_block_count(dev_num);

	if (!strcmp(argv[1], "create"))
		new_partitions(disk, dev_num);
	else if (!strcmp(argv[1], "print"))
		list_table(disk, dev_num);
	else
		usage();

	return 0;
}

U_BOOT_CMD(
	mbr, 3, 0, do_mbr,
	"mbr		mbr/ebr for mmc.\n",
	"create <inand or sdcard>	create mbr\n"
	"mbr print <inand or sdcard>	print mbr\n"
);
