CROSS_COMPILE = arm-linux-

CC = $(CROSS_COMPILE)gcc
STRIP = $(CROSS_COMPILE)strip

CFLAGS = -static -lrt

TARGETS = low-power-mode klogwrapper

ALL: $(TARGETS)

%: %.c
	$(CC) $< -o $@ $(CFLAGS)
	$(STRIP) $@
