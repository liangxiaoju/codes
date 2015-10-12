CC		?= arm-linux-gcc

CFLAGS	= $(shell pkg-config --cflags --libs libpng)

prog	= png2raw rgb

ALL:	$(prog)

%:	%.c
		 $(CC) $(CFLAGS) $< -o $@
