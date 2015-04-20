#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <curses.h>
#include <cdk.h>
#include "win.h"

int init_barwin(barwin_t *barwin)
{
	box(barwin->win, 0, 0);
	barwin->screen = initCDKScreen(barwin->win);
	barwin->buttonbox = newCDKButtonbox(barwin->screen,
			0, 0, 0, 0,
			"<C></B/24>TAG", 1, barwin->nbutton,
			barwin->buttonList, barwin->nbutton,
			A_REVERSE, TRUE, FALSE);
	setCDKButtonboxBackgroundColor(barwin->buttonbox, "</68>");
	refreshCDKScreen(barwin->screen);
	return 0;
}

int init_selectwin(selectwin_t *selectwin, int iselect)
{
	char *list[] = {" "};
	int lines, cols;

//	box(selectwin->win, 0, 0);
	selectwin->screen = initCDKScreen(selectwin->win);
	getmaxyx(selectwin->win, lines, cols);
	selectwin->selectionList = newCDKSelection(
			selectwin->screen, CENTER, CENTER, LEFT, lines, cols,
			"<C></B/24>Selection",
			(CDK_CSTRING2)selectwin->itemList, selectwin->nchoise,
			(CDK_CSTRING2)list, 1,
			A_REVERSE, TRUE, FALSE);
//	setCDKSelectionBackgroundColor(selectwin->selectionList, "</41>");

//	refreshCDKScreen(selectwin->screen);
	return 0;
}

int init_boxwin(boxwin_t *boxwin, int ibox)
{
	box(boxwin->win, 0, 0);
	boxwin->screen = initCDKScreen(boxwin->win);
	wbkgd(boxwin->win, COLOR_PAIR(5));
//	refreshCDKScreen(boxwin->screen);
	return 0;
}

int init_tagwin(tagwin_t *tagwin, int itag)
{
	selectwin_t *selectwin;
	boxwin_t *boxwin;
	int lines, cols;

//	box(tagwin->win, 0, 0);
//	wbkgd(tagwin->win, COLOR_PAIR(68));
	tagwin->screen = initCDKScreen(tagwin->win);
//	refreshCDKScreen(tagwin->screen);

	selectwin = &tagwin->selectwin;
	getmaxyx(tagwin->win, lines, cols);
	selectwin->win = derwin(tagwin->win, lines, cols/4, 0, 0);
	init_selectwin(selectwin, itag);

	boxwin = &tagwin->boxwin;
	boxwin->win = derwin(tagwin->win, lines, cols - cols/4, 0, cols/4);
	init_boxwin(boxwin, itag);
	return 0;
}

int init_mainwin(mainwin_t *mainwin)
{
	int i, lines, cols;

	mainwin->win = initscr();

	initCDKColor();
	init_pair(65, COLOR_RED, COLOR_BLUE);
	init_pair(66, COLOR_RED, COLOR_CYAN);
	init_pair(67, COLOR_RED, COLOR_GREEN);
	init_pair(68, COLOR_BLACK, COLOR_WHITE);

//	box(mainwin->win, 0, 0);

	/* XXX main window color: blue */
	wbkgd(mainwin->win, COLOR_PAIR(5));
	wrefresh(mainwin->win);

	mainwin->screen = initCDKScreen(mainwin->win);
//	refreshCDKScreen(mainwin->screen);

	getmaxyx(mainwin->win, lines, cols);
	mainwin->bar.win = derwin(mainwin->win, 4, cols, 0, 0);
	init_barwin(&mainwin->bar);

//	mainwin->tag[0].win = derwin(mainwin->win, LINES-6, COLS-2, 5, 1);
	for (i = 0; i < mainwin->ntag; i++) {
//		mainwin->tag[i].win = mainwin->tag[0].win;
		mainwin->tag[i].win = derwin(mainwin->win, lines-4, cols, 4, 0);
		init_tagwin(&mainwin->tag[i], i);
	}

	return 0;
}

int default_handler(WINDOW *win, void *ptr)
{
	char *mesg[] = {"<C>Warning !!!", "<C>No defined handler for this module."};
	CDKSCREEN *screen;

	screen = initCDKScreen(win);
	popupLabel(screen, (CDK_CSTRING2)mesg, 2);
	eraseCDKScreen(screen);
	destroyCDKScreen(screen);

	return 0;
}

int winmain(int argc, const char **argv)
{
	mainwin_t mainwin;
	mainwin_t *pmainwin = &mainwin;
	int iselect = 0, itag = 0;
	int key = 0;
	int nmodules;

	memset(pmainwin, 0, sizeof(mainwin_t));

	if ((nmodules = get_modules(pmainwin)) <= 0)
		return -1;

	init_mainwin(pmainwin);

	do {
		barwin_t *barwin;
		tagwin_t *tagwin;
		selectwin_t *selectwin;
		boxwin_t *boxwin;
		CDKBUTTONBOX *buttonbox;
		CDKSELECTION *selectionList;
		handler_t handler;
		void *ptr;

		barwin = &pmainwin->bar;
		buttonbox = barwin->buttonbox;

		tagwin = &pmainwin->tag[itag];
		selectwin = &tagwin->selectwin;
		boxwin = &tagwin->boxwin;
		selectionList = selectwin->selectionList;

		refreshCDKScreen(tagwin->screen);
		refreshCDKScreen(selectwin->screen);
		refreshCDKScreen(boxwin->screen);

		do {
			activateCDKSelection(selectionList, 0);
			if (selectionList->exitType == vESCAPE_HIT)
				break;
			iselect = getCDKSelectionCurrent(selectionList);

			ptr = pmainwin;
			handler = selectwin->choise[iselect].handler;
			if (handler)
				handler(boxwin->win, ptr);
			else
				default_handler(boxwin->win, ptr);

			touchwin(boxwin->win);
			wrefresh(boxwin->win);

		} while (1);

		activateCDKButtonbox(buttonbox, 0);
		if (buttonbox->exitType == vESCAPE_HIT)
			break;
		itag = getCDKButtonboxCurrentButton(buttonbox);

	} while (1);

	endCDK();
//	printf("iselect=%d itag=%d\n", iselect, itag);

	return 0;
}

int main(int argc, const char *argv[])
{
	pid_t	pid;
	int status;

	pid = fork();
	if (pid == 0) {
		winmain(argc, argv);
	} else {
		while (1) {
			waitpid(pid,  &status, 0);
			if (WIFEXITED(status))
				break;

			printf("ERROR:%d, restart win\n", status);
			pid = fork();
			if (pid == 0)
				winmain(argc, argv);
		}
	}

	return 0;
}

