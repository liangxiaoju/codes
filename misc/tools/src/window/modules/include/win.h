#ifndef __WIN_H__
#define __WIN_H__

#include "module.h"
#include "msg.h"

#define MAX_ITEM (20)
#define MAX_TAG (10)
#define MAX_MODULE (MAX_ITEM * MAX_TAG)

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

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
	WINDOW *win;
	CDKSCREEN *screen;

	barwin_t bar;
	tagwin_t tag[MAX_TAG];
	int ntag;

	int curr;

	module_t *modules[MAX_MODULE];
	int nmodules;

	struct message msg_list;

} mainwin_t;


int get_modules(mainwin_t *mainwin);

#endif
