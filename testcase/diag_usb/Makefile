prog	= diag_usb
CFLAGS	= $(shell pkg-config --cflags --libs libusb-1.0)

files	= diag_usb.c diagchar_hdlc.c crc-ccitt.c usb.c

$(prog): $(files)
	$(CC) $(CFLAGS) -o $@ $(files)

clean:
	rm -f *.o
	rm -f $(prog)

