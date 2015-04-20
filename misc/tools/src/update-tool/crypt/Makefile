CROSS_COMPILE	= arm-linux-
CC		= $(CROSS_COMPILE)gcc
STRIP		= $(CROSS_COMPILE)strip
CFLAGS	= -static -Wall

all: encrypt decrypt

encrypt:	*.c
	gcc $(CFLAGS) $< -o $@
	strip $@

decrypt:	*.c
	$(CC) $(CFLAGS) $< -o $@
	$(STRIP) $@

clean:
	rm -rf encrypt decrypt
