#include <panel.h>

int main()
{
	int i;
	int ch;
	int row, col;
	int nlines = 10, ncols = 30, y = 5, x = 5;
	WINDOW *win[3];
	PANEL *panel[3], *top;

	initscr();
	raw();
	keypad(stdscr, TRUE);
	noecho();
	printw("------ test ---------\n");
	refresh();

	for (i = 0; i < 3; i++) {
		win[i] = newwin(nlines, ncols, y + i, x + i);
		box(win[i], 0, 0);
		wprintw(win[i], "win%d", i);
		panel[i] = new_panel(win[i]);
	}
	update_panels();
	doupdate();

	i = 0;
	while ((ch = getch()) != KEY_F(1)) {
		if (ch == 9) {
			top = panel[i];
			i = (++i) % 3;
			top_panel(top);
		}
		update_panels();
		doupdate();
	}

	endwin();
	return 0;
}
