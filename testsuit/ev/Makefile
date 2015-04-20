CC := gcc
FLAGS := -Iinclude -pthread

sfile := \
	srv/srv.c \
	srv/server.c \
	srv/client.c \
	srv/evdev.c \
	srv/dispatch.c \
	common/socket.c \
	common/loop.c

all: server client

server: $(sfile)
	$(CC) $(FLAGS) $(sfile) -o $@

cfile := \
	cli/cli.c \
	cli/client.c \
	common/socket.c \
	common/loop.c

client: $(cfile)
	$(CC) $(FLAGS) $(cfile) -o $@
