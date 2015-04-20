CC		:= gcc
CFLAGS	= -shared -fPIC -Iinclude -lcdkw -lncursesw -lpthread -Wall -ltinyalsa

TARGETS	= libkeyboard.so libsound.so libwifi.so libevent.so libsdcard.so


ALL:	$(TARGETS)

lib%.so:	%.c
	$(CC) -o $@ $< $(CFLAGS)

clean:
	rm -f *.so
