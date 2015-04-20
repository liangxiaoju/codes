#ifndef RKROM_29XX_H
#define RKROM_29XX_H

#pragma pack(1)

// a as major version number, b as minor version number, c as revision number
#define ROM_VERSION(a,b,c) (((a) << 24) + ((b) << 16) + (c))

#define RK_ROM_HEADER_CODE "RKFWf"
struct _rkfw_header //for rk2818
{
	char	head_code[4];	// "RKFW"
	unsigned short head_len; //0x0066
	unsigned int	version;	// 0x00020003
	unsigned int	code;  //0x01030000

	// 创建时间
	unsigned short	year;
	unsigned char	month;
	unsigned char	day;
	unsigned char	hour;
	unsigned char	minute;
	unsigned char	second;
	
	unsigned int	chip;	// 芯片类型 0x00000021
	
	unsigned int	loader_offset;	//loader 偏移0x00000066
	unsigned int	loader_length;	//loader 长度0x000235ca

	unsigned int	image_offset;		//image偏移0x00023630
	unsigned int	image_length;		//image长度0x108e4804	

	unsigned int unknown1;      //0x00000000
	unsigned int unknown2; //0x00000001
	unsigned int system_fstype; //0x00000000
	unsigned int backup_endpos; //0x00000024
	
	unsigned char reserved[0x2D];
};

struct bootloader_header {
	char magic[4];
	unsigned short head_len;
	unsigned int version;
	unsigned int unknown1;

	unsigned short build_year;
	unsigned char build_month;
	unsigned char build_day;
	unsigned char build_hour;
	unsigned char build_minute;
	unsigned char build_second;
	/* 104 (0x68) bytes */

	unsigned int chip;
};

#pragma pack()

#endif // RKROM_29XX_H
