CC		:= gcc
FLAGS	= -lcdkw -lncursesw -rdynamic -ldl -Iinclude
prog	= win

file	= win.c module.c

$(prog): $(file)
	$(CC) $(FLAGS) $(file) -o $@

clean:
	rm -f $(prog)
