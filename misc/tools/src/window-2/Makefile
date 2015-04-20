CC		:= gcc
FLAGS	= -lcdkw -lncursesw -lpthread -rdynamic -ldl -Iinclude
prog	= win

file	= module.c thread.c event.c win.c

$(prog): $(file)
	$(CC) $(FLAGS) $(file) -o $@

clean:
	rm -f $(prog)
