#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include "win.h"
#include "event.h"

mainwin_t g_mainwin;

int init_bar(barwin_t *barwin)
{
	box(barwin->win, 0, 0);
	mvwprintw(barwin->win, 1, COLS/2-5, "Test Mode");
	wrefresh(barwin->win);

	return 0;
}

int init_subwin(subwin_t *subwin)
{
	sbarwin_t *sbarwin = &subwin->sbarwin;
	modwin_t *modwin = &subwin->modwin;
	int lines, cols;

	getmaxyx(subwin->win, lines, cols);

	sbarwin->win = derwin(subwin->win, 1, cols, 0, 0);
	modwin->win = derwin(subwin->win, lines-1, cols, 1, 0);

	return 0;
}

int init_box(boxwin_t *boxwin)
{
	touchwin(boxwin->win);
	wrefresh(boxwin->win);
	return 0;
}

int init_key(mainwin_t *mainwin)
{
	define_key("\033[19~", KEY_HOME);
	define_key("\033[20~", KEY_OPTIONS);
	define_key("\033[[E", KEY_REFRESH);
	define_key("\033[5~", KEY_PPAGE);
	define_key("\033[6~", KEY_NPAGE);
	define_key("\033[P", KEY_SUSPEND);
	define_key("\033[21~", KEY_SSUSPEND);
	return 0;
}

int init_mainwin(mainwin_t *mainwin)
{
	int line, col;
	int lines, cols;
	int height, width;
	int i;
	int parts = 4;

	col = 2;
	line = 2;

	mainwin->win = initscr();

	mainwin->bar.win = derwin(mainwin->win, 4, COLS, 0, 0);
	init_bar(&mainwin->bar);

	mainwin->box.win = derwin(mainwin->win, LINES-4, COLS, 4, 0);
	init_box(&mainwin->box);

	getmaxyx(mainwin->box.win, lines, cols);
	width = cols / col;
	height = lines / line;
/*
	mainwin->box.groups = assign_subwin(mainwin);
	mainwin->box.curr = -1;
*/
	for (i = 0; i < mainwin->box.nsubs; i++) {
		boxwin_t *box = &mainwin->box;
		subwin_t *subwin = &box->subwin[i];
		subwin->win = derwin(box->win,
				height, width, ((i%parts)/col)*height, ((i%parts)%col)*width);
		subwin->boxwin = box;
		subwin->id = i;
		subwin->group = i/parts;
		init_subwin(subwin);
	}

	mainwin->box.curr = -1;
	mainwin->box.groups = (mainwin->box.nsubs + parts -1) / parts;

	init_key(mainwin);

	return 0;
}

int start_pthread(mainwin_t *mainwin, int igroup)
{
	int i;
	int lines, cols;
	int len;
	pthread_t *tid;
	handler_t handler;
	modwin_t *modwin;
	subwin_t *subwin;

	for (i = 0; i < mainwin->box.nsubs; i++) {

		subwin = &mainwin->box.subwin[i];

		if (igroup == subwin->group) {

			modwin = &subwin->modwin;
			tid = &subwin->tid;
			handler = subwin->module->handler;
			if (!handler)
				continue;

			window_lock();

			getmaxyx(subwin->sbarwin.win, lines, cols);
			len = strlen(subwin->module->name);
			mvwprintw(subwin->sbarwin.win, 0, cols/2-len/2, "%s", subwin->module->name);

			box(subwin->modwin.win, 0, 0);

			wnoutrefresh(subwin->win);
			doupdate();

			window_unlock();

			pthread_create(tid, NULL, handler, modwin);
		}
	}
	mainwin->box.igroup = igroup;

	return 0;
}

int stop_pthread(mainwin_t *mainwin, int igroup)
{
	int i;
	pthread_t tid;
	subwin_t *subwin;

	for (i = 0; i < mainwin->box.nsubs; i++) {

		subwin = &mainwin->box.subwin[i];

		if (igroup == subwin->group) {

			tid = subwin->tid;
			cancel_subwin(&subwin->modwin);
			pthread_join(tid, NULL);

			window_lock();
			werase(subwin->win);
			wnoutrefresh(subwin->win);
			window_unlock();
		}
	}

	return 0;
}

int get_group_members(boxwin_t *box, int igroup)
{
	int i;
	int num = 0;
	subwin_t *subwin;

	for (i = 0; i < box->nsubs; i++) {
		subwin = &box->subwin[i];
		if (igroup == subwin->group)
			num++;
	}

	return num;
}

int get_subwin_index(int index, int igroup)
{
	return index + 4*igroup;
}

int handle_loop(mainwin_t *mainwin)
{
	int i;
	int key = 0;
	int index = 0;
	int index_actual = 0;
	int igroup = 0;
	int x = 0, y = 0;
	int lines, cols;
	int num, len;
	boxwin_t *box = &mainwin->box;
	subwin_t *subwin;
	sbarwin_t *sbarwin;
	int exit = 0;

	start_pthread(mainwin, igroup);

	for (;;) {

		window_lock();

		index_actual = get_subwin_index(index, igroup);

		subwin = &box->subwin[index_actual];
		sbarwin = &subwin->sbarwin;

		len = strlen(subwin->module->name);
		getmaxyx(sbarwin->win, lines, cols);

		wattron(sbarwin->win, A_REVERSE);
		mvwprintw(sbarwin->win, 0, cols/2-len/2, "%s", subwin->module->name);
		wattroff(sbarwin->win, A_REVERSE);
		wrefresh(sbarwin->win);

		window_unlock();

		key = get_event(NULL);

		if (key == 0x1b) {
			exit++;
			if (exit >= 5)
				break;
		} else
			exit = 0;

		if (key == 0xa) {
			enter_subwin(&box->subwin[index_actual].modwin);
		}

		i = index;

		if (key == KEY_RIGHT)
			i++;
		if (key == KEY_LEFT)
			i--;
		if (key == KEY_UP)
			i -= 2;
		if (key == KEY_DOWN)
			i += 2;

		num = get_group_members(box, igroup);
		if (!(i < 0 || i >= num))
			index = i;

		window_lock();
		mvwprintw(sbarwin->win, 0, cols/2-len/2, "%s", subwin->module->name);
		wnoutrefresh(sbarwin->win);
		window_unlock();

		if (key == KEY_NPAGE) {
			int groups = mainwin->box.groups - 1;
			stop_pthread(mainwin, igroup);
			igroup = (igroup+1 > groups) ? groups : igroup+1;
			start_pthread(mainwin, igroup);
			index = 0;
		} else if (key == KEY_PPAGE) {
			stop_pthread(mainwin, igroup);
			igroup = (igroup-1 < 0) ? 0 : igroup-1;
			start_pthread(mainwin, igroup);
			index = 0;
		}
	}

	return 0;
}

int exit_mainwin(mainwin_t *mainwin)
{
	endCDK();
	endwin();
	return 0;
}

int main(int argc, char *argv[])
{
	mainwin_t *mainwin = &g_mainwin;
	int nmodules;

	memset(mainwin, 0, sizeof(mainwin_t));

	window_lock_init();

	if ((nmodules = get_modules(mainwin)) < 0)
		return -1;

	event_init(mainwin);

	init_mainwin(mainwin);

	handle_loop(mainwin);

	exit_mainwin(mainwin);

	return 0;
}
