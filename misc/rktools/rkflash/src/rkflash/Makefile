prog	= rk_flash
CFLAGS	= -ggdb3 $(shell pkg-config --cflags --libs libusb-1.0)


$(prog): rk_flash.c
	$(CC) $(CFLAGS) $< -o $@

