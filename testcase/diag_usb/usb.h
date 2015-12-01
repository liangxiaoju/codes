#ifndef __USB_H__
#define __USB_H__

#include <libusb.h>

struct usb_handle {
	libusb_context *ctx;
	libusb_device_handle *dev;
};

extern struct usb_handle *usb_init(void);
extern void usb_exit(struct usb_handle *handle);
extern int usb_send(struct usb_handle *handle, unsigned char *buf, int size);
extern int usb_recv(struct usb_handle *handle, unsigned char *buf, int size);

#endif
