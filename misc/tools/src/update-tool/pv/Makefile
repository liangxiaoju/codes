prog	= pv
CROSS_COMPILE	= arm-linux-
CC		= $(CROSS_COMPILE)gcc
CFLAGS	= -static -Wall

$(prog):	*.c
	$(CC) $(CFLAGS) $< -o $@
