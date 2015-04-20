#include <curses.h>
#include <cdk.h>

#define MAX_ITEM 20

typedef int (*func_t)(WINDOW *win);

typedef struct {
	char *name;
	func_t func;
} choise_t;

typedef struct {

	WINDOW *cursesWin;

	WINDOW *subWin;

	CDKSCREEN *cdkScreen;

	CDKLABEL *titleLable;
	const char *titleMesg[4];

	CDKDIALOG *dialog;

	CDKSELECTION *selectionList;
	choise_t choise[MAX_ITEM];
	char *itemList[MAX_ITEM];

} screen_t;

int get_items(screen_t *pscreen)
{
	pscreen->choise[0].name = pscreen->itemList[0] = copyChar("item0");
	pscreen->choise[1].name = pscreen->itemList[1] = copyChar("item1");
	pscreen->choise[2].name = pscreen->itemList[2] = copyChar("item2");
	return 3;
}

int handle_selection(screen_t *pscreen, int n)
{
	mvwprintw(pscreen->subWin, 1, 1, "select %d", n);
	wrefresh(pscreen->subWin);
	return 0;
}

int main(int argc, char *argv[])
{
	screen_t screen, *pscreen;

	int iselect, nitem;

	pscreen = &screen;

	pscreen->cursesWin = initscr();
	pscreen->subWin = derwin(pscreen->cursesWin, 0, 0, 2, 41);
	box(pscreen->subWin, 0, 0);

	pscreen->cdkScreen = initCDKScreen(pscreen->cursesWin);

	pscreen->titleMesg[0] = copyChar("<C></U>Title");
	pscreen->titleLable = newCDKLabel(pscreen->cdkScreen, CENTER, 1,
			(CDK_CSTRING2)pscreen->titleMesg, 1,
			FALSE, FALSE);

	refreshCDKScreen(pscreen->cdkScreen);

	nitem = get_items(pscreen);
	pscreen->selectionList = newCDKSelection(pscreen->cdkScreen, 1, 2, NONE,
			LINES-2, 40,
			"<C></U>Selection:",
			(CDK_CSTRING2)pscreen->itemList, nitem,
			(CDK_CSTRING2)copyChar(""), 1,
			A_REVERSE, TRUE, FALSE);

	do {
		activateCDKSelection(pscreen->selectionList, 0);

		iselect = getCDKSelectionCurrent(pscreen->selectionList);

		handle_selection(pscreen, iselect);

	} while (pscreen->selectionList->exitType != vESCAPE_HIT);

	getch();

	destroyCDKLabel(pscreen->titleLable);

	endCDK();

	return 0;
}
