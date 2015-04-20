prog	= rkflash
CFLAGS	= -ggdb3 $(shell pkg-config --cflags --libs libusb-1.0)


$(prog): rkflash.c
	$(CC) $(CFLAGS) $< -o $@

