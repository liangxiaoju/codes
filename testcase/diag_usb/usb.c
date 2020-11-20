#include <stdio.h>
#include <stdlib.h>
#include "usb.h"

#define EP_OUT	0x01
#define EP_IN	0x81

struct usb_handle *usb_init(void)
{
	struct usb_handle *handle;

	handle = malloc(sizeof(*handle));
	if (!handle) {
		perror("malloc");
		return NULL;
	}

	libusb_init(&(handle->ctx));
	libusb_set_debug(handle->ctx, 4);

    /*
	handle->dev = libusb_open_device_with_vid_pid(
			handle->ctx, 0x1bbb, 0x9018);
    */
	handle->dev = libusb_open_device_with_vid_pid(
			handle->ctx, 0x05c6, 0x9091);
	if (!handle->dev) {
		fprintf(stderr, "Failed to open usb device.\n");
		free(handle);
		return NULL;
	}

	if (libusb_kernel_driver_active(handle->dev, 0)) {
		fprintf(stderr, "Try to detach driver\n");
		if (libusb_detach_kernel_driver(handle->dev, 0)) {
			fprintf(stderr, "Try to reset device\n");
			libusb_reset_device(handle->dev);
		}
	}

	if (libusb_claim_interface(handle->dev, 0)) {
		libusb_reset_device(handle->dev);
		if (libusb_claim_interface(handle->dev, 0)) {
			perror("libusb_claim_interface");
			free(handle);
			return NULL;
		}
	}

	return handle;
}

void usb_exit(struct usb_handle *handle)
{
	if (handle) {
		libusb_release_interface(handle->dev, 0);
		libusb_close(handle->dev);
		libusb_exit(handle->ctx);
		free(handle);
	}
}

int usb_send(struct usb_handle *handle, unsigned char *buf, int size)
{
	int len;

	if (libusb_bulk_transfer(handle->dev, EP_OUT, buf, size, &len, 0) < 0) {
		perror("usb_send");
		return -1;
	}

	if (len != size) {
		fprintf(stderr, "size: %d, len: %d\n", size, len);
		return -1;
	}

	return len;
}

int usb_recv(struct usb_handle *handle, unsigned char *buf, int size)
{
	int len;

	if (libusb_bulk_transfer(handle->dev, EP_IN, buf, size, &len, 0) < 0) {
		perror("usb_recv");
		return -1;
	}

	return len;
}

