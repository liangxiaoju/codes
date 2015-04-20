#include <curses.h>
#include <ncursesw/panel.h>
#include <ncursesw/menu.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

typedef int (*choice_func_t)(WINDOW *win);

struct choice {
	char *name;
	choice_func_t func;
};

int default_func(WINDOW *win)
{
	WINDOW *swin;
	swin = subwin(win, LINES/4, COLS/4, LINES*3/8, COLS*3/8);
	box(swin, 0, 0);
	mvwprintw(swin, 1, 1,"selectd");

	wrefresh(swin);
	getch();
	return 0;
}

struct choice choices[] = {
	{ "item1", default_func },
	{ "item2", default_func },
	{ "item3", default_func },
	{ "item4", default_func },
	{ "item5", default_func },
};

int load_module(void)
{
	void *handle;
	choice_func_t p;
}

int main(int argc, char *argv[])
{
	int i, nchoices;
	int ch;

	PANEL *panel[2], *top;
	WINDOW *win[2];
	MENU *menu;
	ITEM **items;

	initscr();
	start_color();
	raw();
	noecho();
	keypad(stdscr, TRUE);
	init_pair(1, COLOR_RED, COLOR_BLACK);
	init_pair(2, COLOR_GREEN, COLOR_BLACK);
	init_pair(3, COLOR_MAGENTA, COLOR_BLACK);

	for (i = 0; i < 2; i++) {
		win[i] = newwin(LINES, COLS, 0, 0);
		box(win[i], 0, 0);
		wprintw(win[i], "win%d", i);
		panel[i] = new_panel(win[i]);
	}
	top = panel[0];
	top_panel(top);
	update_panels();
	doupdate();
	refresh();

	nchoices = ARRAY_SIZE(choices);
	items = (ITEM **)calloc(nchoices+1, sizeof(ITEM *));
	for (i = 0; i < nchoices; i++) {

		int l = COLS - 6 - strlen(choices[i].name);
		char *desc = calloc(1, l);
		memset(desc, ' ', l);
		desc[l-1] = 0;

		items[i] = new_item(choices[i].name, desc);
		set_item_userptr(items[i], choices[i].func);
	}

	items[nchoices] = (ITEM *)NULL;
	menu = new_menu((ITEM **)items);
	set_menu_win(menu, subwin(win[0], 0, 0, 0, 0));
	set_menu_sub(menu, derwin(menu_win(menu), 0, 0, 1, 1));
//	set_menu_format(menu, 10, 1);
	set_menu_mark(menu, "--> ");
	post_menu(menu);
	touchwin(menu_win(menu));
	wrefresh(menu_win(menu));

	while ((ch = getch()) != KEY_F(1)) {
		switch (ch) {
			case KEY_DOWN:
				menu_driver(menu, REQ_DOWN_ITEM);
				wrefresh(menu_sub(menu));
				break;
			case KEY_UP:
				menu_driver(menu, REQ_UP_ITEM);
				wrefresh(menu_sub(menu));
				break;
			case 10:
			{
				ITEM *cur;
				choice_func_t p;
				show_panel(panel[1]);
				update_panels();
				doupdate();
				cur = current_item(menu);
				p = item_userptr(cur);
				p(win[1]);
				pos_menu_cursor(menu);
				show_panel(panel[0]);
				update_panels();
				doupdate();
				break;
			}
		}
	}

	unpost_menu(menu);
	for (i = 0; i < ARRAY_SIZE(choices); i++) {
		free_item(items[i]);
	}
	free_menu(menu);
	endwin();
	return 0;
}
