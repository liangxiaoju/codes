CC		:= gcc
CFLAGS	= -shared -fPIC -Iinclude -lcdkw -lncursesw -Wall

TARGETS	= libsummary.so libautorun.so libwifi.so libsendmsg.so \
		  libpower.so libevent.so libkeyboard.so libsdcard.so


ALL:	$(TARGETS) libsound.so

lib%.so:	%.c
	$(CC) -o $@ $< $(CFLAGS)

libsound.so:	sound.c
	$(CC) -o $@ $< $(CFLAGS) -ltinyalsa

clean:
	rm -f *.so
