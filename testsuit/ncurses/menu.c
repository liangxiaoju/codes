#include <curses.h>
#include <menu.h>
#include <stdlib.h>

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

char *choices[] = {
	"choice 1",
	"choice 2",
	"choice 3",
	"choice 4",
	"choice 5",
	"choice 6",
	"choice 7",
	"exit",
};

void func(char *name)
{
	move(20, 0);
	clrtoeol();
	mvprintw(20, 0, "select: %s", name);
}

int main()
{
	ITEM **items;
	int c;
	MENU *menu;
	int nchoices, i;
	ITEM *curitem;

	initscr();
	start_color();
	raw();
	noecho();
	keypad(stdscr, TRUE);
	init_pair(1, COLOR_RED, COLOR_BLACK);
	init_pair(2, COLOR_GREEN, COLOR_BLACK);
	init_pair(3, COLOR_MAGENTA, COLOR_BLACK);

	nchoices = ARRAY_SIZE(choices);
	items = (ITEM **)calloc(nchoices+1, sizeof(ITEM *));
	for (i = 0; i< nchoices; i++) {
		items[i] = new_item(choices[i], choices[i]);
		set_item_userptr(items[i], func);
	}
	items[nchoices] = (ITEM *)NULL;

	menu = new_menu((ITEM **)items);

	mvprintw(LINES-3, 0, "press ENTER to see option");
	mvprintw(LINES-2, 0, "UP and DOWN");
	post_menu(menu);
	refresh();

	while((c = getch()) != KEY_F(1)) {
		switch(c) {
			case KEY_DOWN:
				menu_driver(menu, REQ_DOWN_ITEM);
				break;
			case KEY_UP:
				menu_driver(menu, REQ_UP_ITEM);
				break;
			case 10:
			{
				ITEM *cur;
				void (*p)(char *);

				cur = current_item(menu);
				p = item_userptr(cur);
				p((char *)item_name(cur));
				pos_menu_cursor(menu);
				break;
			}
		}
	}
	unpost_menu(menu);
	for (i = 0; i < nchoices; i++) {
		free_item(items[i]);
	}

	free_menu(menu);
	endwin();
	return 0;
}
