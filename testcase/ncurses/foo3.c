#include <curses.h>
#include <cdk.h>
#include <dirent.h>
#include <dlfcn.h>

#define MAX_ITEM (20)
#define MAX_TAG (20)
#define LIBS_DIR "/tmp/libs"
#define MODULE_INFO_SYM "module_info_sym"

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

typedef int (*handler_t)(WINDOW *win, void *ptr);

typedef struct {
	char *name;
	handler_t handler;
} choise_t;

typedef struct {
	WINDOW *win;
	CDKSCREEN *screen;

} boxwin_t;

typedef struct {
	WINDOW *win;
	CDKSCREEN *screen;

	CDKSELECTION *selectionList;
	choise_t choise[MAX_ITEM];
	char *itemList[MAX_ITEM];
	int nchoise;

	int curr;
} selectwin_t;

typedef struct {
	WINDOW *win;
	CDKSCREEN *screen;

	selectwin_t selectwin;
	boxwin_t boxwin;
} tagwin_t;

typedef struct {
	WINDOW *win;
	CDKSCREEN *screen;

	CDKBUTTONBOX *buttonbox;
	char *buttonList[MAX_TAG];
	int nbutton;
} barwin_t;

typedef struct {
	int version;
	int tag;
	const char *name;

	handler_t handler;
} module_t;

typedef struct {
	WINDOW *win;
	CDKSCREEN *screen;

	barwin_t bar;
	tagwin_t tag[MAX_TAG];
	int ntag;

	int curr;
} mainwin_t;

int init_barwin(barwin_t *barwin)
{
	box(barwin->win, 0, 0);
	barwin->screen = initCDKScreen(barwin->win);
	barwin->buttonbox = newCDKButtonbox(barwin->screen,
			0, 0, 0, 0,
			"<C></B/24>TAG", 1, barwin->nbutton,
			barwin->buttonList, barwin->nbutton,
			A_REVERSE, TRUE, FALSE);
	refreshCDKScreen(barwin->screen);
	return 0;
}

int init_selectwin(selectwin_t *selectwin, int iselect)
{
	char *list[] = {" "};

	box(selectwin->win, 0, 0);
	selectwin->screen = initCDKScreen(selectwin->win);
	selectwin->selectionList = newCDKSelection(
			selectwin->screen, CENTER, CENTER, LEFT, LINES-10, COLS/8-3,
			"<C></B/24>Selection:",
			(CDK_CSTRING2)selectwin->itemList, selectwin->nchoise,
			(CDK_CSTRING2)list, 1,
			A_REVERSE, FALSE, FALSE);

	refreshCDKScreen(selectwin->screen);
	return 0;
}

int init_boxwin(boxwin_t *boxwin, int ibox)
{
	box(boxwin->win, 0, 0);
	boxwin->screen = initCDKScreen(boxwin->win);
	refreshCDKScreen(boxwin->screen);
	return 0;
}

int init_tagwin(tagwin_t *tagwin, int itag)
{
	selectwin_t *selectwin;
	boxwin_t *boxwin;

	box(tagwin->win, 0, 0);
	tagwin->screen = initCDKScreen(tagwin->win);
	refreshCDKScreen(tagwin->screen);

	selectwin = &tagwin->selectwin;
	selectwin->win = derwin(tagwin->win, LINES-8, COLS/8, 1, 1);
	init_selectwin(selectwin, itag);

	boxwin = &tagwin->boxwin;
	boxwin->win = derwin(tagwin->win, LINES-8, COLS*7/8-3, 1, COLS/8+1);
	init_boxwin(boxwin, itag);
	return 0;
}

int init_mainwin(mainwin_t *mainwin)
{
	int i;

	mainwin->win = initscr();
	initCDKColor();
	box(mainwin->win, 0, 0);
	mainwin->screen = initCDKScreen(mainwin->win);
	refreshCDKScreen(mainwin->screen);

	mainwin->bar.win = derwin(mainwin->win, 4, COLS-2, 1, 1);
	init_barwin(&mainwin->bar);

//	mainwin->tag[0].win = derwin(mainwin->win, LINES-6, COLS-2, 5, 1);
	for (i = 0; i < mainwin->ntag; i++) {
//		mainwin->tag[i].win = mainwin->tag[0].win;
		mainwin->tag[i].win = derwin(mainwin->win, LINES-6, COLS-2, 5, 1);
		init_tagwin(&mainwin->tag[i], i);
	}

	return 0;
}

int default_handler(WINDOW *win, void *ptr)
{
	mvwprintw(win, 1, 1, "select %s", ptr);
	wrefresh(win);
	return 0;
}

int parse_config(mainwin_t *mainwin)
{
	char *items[][5] = {{"</U>item1_1", "</U>item1_2", "</U>item1_3", "</U>item1_4", "</U>item1_5"},
		 {"</U>item2_1", "</U>item2_2", "</U>item2_3", "</U>item2_4", "</U>item2_5"},
		 {"</U>item3_1", "</U>item3_2", "</U>item3_3", "</U>item3_4", "</U>item3_5"}};
	char *buttons[] = {"</U>tag1", "</U>tag2", "</U>tag3"};
	tagwin_t *tag;
	selectwin_t *selectwin;
	int i, j;

	mainwin->bar.nbutton = mainwin->ntag = ARRAY_SIZE(buttons);
	for (i = 0; i < mainwin->ntag; i++) {
		mainwin->bar.buttonList[i] = copyChar(buttons[i]);

		tag = &mainwin->tag[i];
		selectwin = &tag->selectwin;
		selectwin->nchoise = ARRAY_SIZE(items[i]);
		for (j = 0; j < selectwin->nchoise; j++) {
			selectwin->choise[j].name = selectwin->itemList[j] = copyChar(items[i][j]);
			selectwin->choise[j].handler = default_handler;
		}
	}

	return 0;
}

int set_module_info(mainwin_t *mainwin, module_t **module, int n)
{
	int i;
	barwin_t *barwin;
	tagwin_t *tagwin;
	selectwin_t *selectwin;
	handler_t handler;
	int ntag[3] = {0, 0, 0};

	barwin = &mainwin->bar;
	barwin->buttonList[0] = copyChar("</U>tag0");
	barwin->nbutton = mainwin->ntag = 1;

	tagwin = &mainwin->tag[0];
	selectwin = &tagwin->selectwin;
	selectwin->nchoise = n;

	for (i = 0; i < n; i++) {
		selectwin->choise[i].name = selectwin->itemList[i] = copyChar(module[i]->name);
		selectwin->choise[i].handler = module[i]->handler;
	}
	return 0;
}

int get_modules(mainwin_t *mainwin)
{
	int ret = 0, i = 0;
	DIR *dirp;
	struct dirent *dirent;
	char libpath[128];
	void *handle;
	module_t *module[64];

	dirp = opendir(LIBS_DIR);
	if (!dirp)
		return -1;
	for (i = 0; i < ARRAY_SIZE(module);) {
		dirent = readdir(dirp);
		if (!dirent) {
			ret = -1;
			break;
		}

		snprintf(libpath, sizeof(libpath), "%s/%s", LIBS_DIR, dirent->d_name);
		handle = dlopen(libpath, RTLD_NOW);
		if (!handle) {
			ret = -1;
			continue;
		}
		module[i] = (module_t *)dlsym(handle, MODULE_INFO_SYM);
		if (!module[i]) {
			ret = -1;
			dlclose(handle);
			continue;
		}
		i++;
	}
	closedir(dirp);

	set_module_info(mainwin, module, i);

	return ret;
}

int main(int argc, char *argv[])
{
	mainwin_t mainwin;
	mainwin_t *pmainwin = &mainwin;
	int iselect = 0, itag = 0;
	int key = 0;

//	parse_config(pmainwin);
	get_modules(pmainwin);
	init_mainwin(pmainwin);

	do {
		barwin_t *barwin;
		tagwin_t *tagwin;
		selectwin_t *selectwin;
		boxwin_t *boxwin;
		CDKBUTTONBOX *buttonbox;
		CDKSELECTION *selectionList;
		handler_t handler;
		char ptr[32];

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

			sprintf(ptr, "%d_%d", itag, iselect);
			handler = selectwin->choise[iselect].handler;
			handler(boxwin->win, ptr);
		} while (1);

		activateCDKButtonbox(buttonbox, 0);
		if (buttonbox->exitType == vESCAPE_HIT)
			break;
		itag = getCDKButtonboxCurrentButton(buttonbox);

	} while (1);

	getch();
	endCDK();
	printf("iselect=%d itag=%d\n", iselect, itag);

	return 0;
}

