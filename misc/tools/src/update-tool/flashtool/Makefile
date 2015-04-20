CROSS_COMPILE	= arm-boeye-linux-gnueabi-
CC		= $(CROSS_COMPILE)gcc
STRIP		= $(CROSS_COMPILE)strip
CFLAGS	= -static -Wall

all: rk_flash blmessage

rk_flash:	rk_flash.c
	$(CC) $(CFLAGS) $< -o $@

blmessage:	blmessage.c
	$(CC) $(CFLAGS) $< -o $@
