#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libusb.h>
#include <stdlib.h>
#include <unistd.h>


#define RK_BPT	0x20
#define RK_BUFLEN	(RK_BPT*512)

#define	RK_BACKUP_OFFSET	0x30000

#define EP_OUT	0x02
#define EP_IN	0x81


struct rk_cmd {
	int	magic;
	int	code;
	int gap1;
	char flag;	/* [12] */
	int cmd;	/* [13] */
	unsigned int offset;	/* [17] */
	char gap2[2];
	unsigned char size;	/* [23] */
	char gap3[7]; /* [30] */
} __attribute__ ((packed));

struct usb_id {
	uint16_t vid;
	uint16_t pid;
};


void Usage(char *s)
{
	fprintf(stderr, "Usage:%s\n"	\
			"-r offset size file.img.bak	\t dump flash\n" 	\
			"-w offset size file.img		\t flash image\n" 	\
			"-e offset size					\t erase flash\n"	\
			"-b		\t reboot device"	\
			"\n", s);
}

int rk_usb_write(struct libusb_device_handle *dev, struct rk_cmd *cmd, unsigned int size, char *buf)
{
	int len;
	static int cid;

	cmd->code = cid++;
	if (libusb_bulk_transfer(dev, EP_OUT, (unsigned char *)cmd, sizeof(*cmd), &len, 0) < 0) {
		fprintf(stderr, "libusb_bulk_transfer\n");
		return -1;
	}
	if (len != sizeof(*cmd)) {
		fprintf(stderr, "cmd_len: %d, len: %d\n", size, len);
		return len;
	}

	if (buf) {
		if (libusb_bulk_transfer(dev, EP_OUT, buf, size, &len, 0) < 0) {
			fprintf(stderr, "libusb_bulk_transfer\n");
			return -1;
		}
		if (len != size) {
			fprintf(stderr, "size: %d, len: %d\n", size, len);
		}

	}

	return len;
}


int rk_usb_read(struct libusb_device_handle *dev, char *buf, int size)
{
	int len;

	if (libusb_bulk_transfer(dev, EP_IN, buf, size, &len, 0) < 0) {
		return -1;
	}
	
	return len;
}

int main(int argc, const char *argv[])
{
	int opt, fd, flag;
	int offset, size, len;
	char buf[RK_BUFLEN];
	char resp[13];
	unsigned int rw;
	struct rk_cmd cmd;
	libusb_device_handle *dev;
	struct usb_id *id;

	struct usb_id usb_id[] = {
		{0x2207, 0x281a},	/* rk2818 */
		{0x2207, 0x290a},	/* rk2918 */
		{0x2207, 0x292a},	/* rk2928 */
		{0x2207, 0x300a},	/* rk3066 */
		{0x2207, 0x300b},	/* rk3168 */
		{0x2207, 0x310b},	/* rk3188 */
		{0x2207, 0x320a},	/* rk3288 */
		{0}
	};

	offset = 0;
	size = 0;
	while ((opt = getopt(argc, argv, "rweb")) != -1) {
		switch (opt) {
			case 'r':	flag = 0;	break;	/* read */
			case 'w':	flag = 1;	break;	/* write */
			case 'e':	flag = 2;	break;	/* erase */
			case 'b':	flag = 3;	break;	/* reboot */
			default:
                                Usage(argv[0]);
                                return -1;
		}
		if (flag != 3) {
			offset = strtol(argv[optind], NULL, 0);
			size = strtol(argv[optind+1], NULL, 0);
			if (offset < 0 || size <= 0) {
				perror("strtol");
				return -1;
			}
		}
	}

	if (flag == 0) {
		fd = STDOUT_FILENO;
		if (argc == 5) {
			fd = open(argv[4], O_RDWR|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
			if (fd < 0) {
				perror("open");
				return -1;
			}
		}
	} else if (flag == 1) {
		fd = STDIN_FILENO;
		if (argc == 5) {
			fd = open(argv[4], O_RDWR);
			if (fd < 0) {
				perror("open");
				return -1;
			}
		}
	}

	libusb_init(NULL);
	id = usb_id;
	while (id->vid) {
		dev = libusb_open_device_with_vid_pid(NULL, id->vid, id->pid);
		if (dev)
			break;
		id++;
	}
	if (!dev) {
		fprintf(stderr, "open device error\n");
		return -1;
	}

	if (libusb_kernel_driver_active(dev, 0)) {
		fprintf(stderr, "Try to detach driver\n");
		if (libusb_detach_kernel_driver(dev, 0)) {
            fprintf(stderr, "Try to reset device\n");
            libusb_reset_device(dev);
		}
	}

	if (libusb_claim_interface(dev, 0)) {
        libusb_reset_device(dev);
	    if (libusb_claim_interface(dev, 0)) {
            perror("libusb_claim_interface");
            return -1;
        }
	}

	/* rk init */
	cmd.magic = 0x43425355;	/* USBC */
	cmd.flag = 0x80;
	cmd.cmd = 0x00000600;
	cmd.size = 0;
	rk_usb_write(dev, &cmd, 0, NULL);
	len = rk_usb_read(dev, resp, sizeof(resp));
	if (len != sizeof(resp)) {
		fprintf(stderr, "rk_usb_read error: %d\n", len);
		return -1;
	}

	sleep(1);

	switch (flag) {
		case 0:
			if (offset > RK_BACKUP_OFFSET)	/* backup partition offset */
				rw = 1;
			cmd.flag = 0x80;
			cmd.cmd = 0x00140a00 | rw << 24;
			cmd.size = RK_BPT;
			while(size > 0) {
				cmd.offset = htonl(offset);
				len = rk_usb_write(dev, &cmd, RK_BUFLEN, 0);
				len = rk_usb_read(dev, buf, sizeof(buf));
				if (write(fd, buf, len) != len) {
					perror("write");
					return -1;
				}
				len = rk_usb_read(dev, resp, sizeof(resp));

				offset += RK_BPT;
				size -= RK_BPT;
			}
			break;
		case 1:
			if (offset > RK_BACKUP_OFFSET)
				rw = 1;
			cmd.flag = 0x00;
			cmd.cmd = 0x00150a00 | rw<<24;
			while (size > 0) {
				fprintf(stderr, "\rremained size: 0x%04x", size);
				memset(buf, 0xff, sizeof(buf));
				len = read(fd, buf, sizeof(buf));
				if(len < 0) {
					perror("read");
					return -1;
				} else if (len == 0) {
					fprintf(stderr, "\nImage flash completed\n");
					break;
				}
				cmd.size = len/512;
				if (len % 512) {
					cmd.size++;
				}
				cmd.offset = htonl(offset);
				len = rk_usb_write(dev, &cmd, (cmd.size * 512) , buf);
				len = rk_usb_read(dev, resp, sizeof(resp));
				if (len != sizeof(resp)) {
					fprintf(stderr, "usb read response error %d\n", len);
					return -1;
				}

				offset += RK_BPT;
				size -= RK_BPT;
			}
			fprintf(stderr, "\n");
			break;
		case 2:	
			break;
		case 3:
			printf("reboot device...\n");
			cmd.flag = 0x00;
			cmd.cmd = 0x00ff0600;
			len = rk_usb_write(dev, &cmd, 0, NULL);
			len = rk_usb_read(dev, resp, sizeof(resp));
			break;
	}


	libusb_release_interface(dev, 0);
	libusb_close(dev);
	libusb_exit(NULL);

	return 0;
}
