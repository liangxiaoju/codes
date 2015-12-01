#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <linux/types.h>
#include <unistd.h>
#include "usb.h"
#include "crc-ccitt.h"
#include "diagchar_hdlc.h"

void hexdump(unsigned char *buf, int len)
{
	int i;

	for (i = 0; i < len; i++) {
		printf("0x%02x ", buf[i]);
	}
	printf("\n");
}

int diag_usb_send_data(int argc, char *argv[])
{
	unsigned char *in, *out;
	int in_len = argc - 1;
	int out_len = in_len*4;
	int actual_len;
	int i;

	in = malloc(in_len);
	if (!in) {
		perror("malloc");
		return -1;
	}

	out = malloc(out_len);
	if (!out) {
		perror("malloc");
		return -1;
	}

	for (i = 0; i < in_len; i++) {
		in[i] = strtoul(argv[i+1], NULL, 0);
	}

	actual_len = diag_usb_encode(in, in_len, out, out_len);

	do {
		struct usb_handle *handle = usb_init();
		if (!handle) {
			return -1;
		}

		printf("%s ", argv[0]);
		hexdump(out, actual_len);

		usb_send(handle, out, actual_len);

		int buf_len = 8*1024;
		char *buf = malloc(buf_len);
		actual_len = usb_recv(handle, buf, buf_len);

		printf("recv: ");
		hexdump(buf, actual_len);

		free(buf);

		usb_exit(handle);
	} while (0);

	free(in);
	free(out);

	return 0;
}

struct diag_usb_ops {
	char *action;
	int (*handler)(int argc, char *argv[]);
} diag_usb_ops[] = {
	{
		.action = "send_data",
		.handler = diag_usb_send_data,
	},
};

void usage(char *argv0)
{
	int i;

	printf("%s [action]\n", argv0);
	printf("  action:\n");
	for (i = 0; i < sizeof(diag_usb_ops)/sizeof(diag_usb_ops[0]); i++) {
		printf("         %s [data]\n", diag_usb_ops[i].action);
	}

	printf("example:\n");
	printf("%s send_data 0x80 0xFA 0x77 0x00 0x00 0x02 0x01\n", argv0);
	printf("%s send_data 0x80 0xFA 0x78 0x00 0x10 0x00 0x03 0xFF 0xFF 0xFF\n", argv0);
}

int main(int argc, char *argv[])
{
	char *action = argv[1];
	int i;

	if (argc == 1) {
		usage(argv[0]);
		return 0;
	}

	for (i = 0; i < sizeof(diag_usb_ops)/sizeof(diag_usb_ops[0]); i++) {
		if (strcmp(action, diag_usb_ops[i].action) == 0) {
			argv[1] = action;
			diag_usb_ops[i].handler(argc-1, argv+1);
		}
	}

	return 0;
}

