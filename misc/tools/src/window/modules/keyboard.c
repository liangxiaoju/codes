#include <stdio.h>
#include <stdlib.h>
#include "module.h"
#include "msg.h"

typedef struct {
	char *name;
	char *value;
	int code;
} mykey_t;

mykey_t mykey[] = {
	{ "KEY_HOME", 		"\033[19~",		KEY_MAX+1 },
	{ "KEY_MENU", 		"\033[20~",		KEY_MAX+2 },
	{ "KEY_REFRESH",	"\033[[E",		KEY_MAX+3 },
	{ "KEY_PAGEUP",		"\033[5~",		KEY_MAX+4 },
	{ "KEY_PAGEDOWN",	"\033[6~",		KEY_MAX+5 },
	{ "KEY_SHORT_POWER","\033[P",		KEY_MAX+6 },
	{ "KEY_LONG_POWER",	"\033[21~",		KEY_MAX+7 },
};

int define_mykey(void)
{
	int i;
	for (i = 0; i < sizeof(mykey)/sizeof(mykey[0]); i++) {
		define_key(mykey[i].value, mykey[i].code);
	}
	return 0;
}

char *mykeyname(int code)
{
	int i;
	for (i = 0; i < sizeof(mykey)/sizeof(mykey[0]); i++) {
		if (mykey[i].code == code)
			return mykey[i].name;
	}
	return 0;
}

int keyboard_handler(WINDOW *win, void *ptr)
{
	CDKSCREEN *screen;
	CDKSWINDOW *swindow;
	char *title = "<C>KeyBoard";
	char *mesg;
	char temp[128];
	int key;

	screen = initCDKScreen(win);
	swindow = newCDKSwindow(screen, CENTER, CENTER,
			0, 0, title, 100, TRUE, FALSE);

	drawCDKSwindow(swindow, TRUE);
	keypad(screen->window, TRUE);
	define_mykey();
	while (1) {
		const char *name;
		key = wgetch(screen->window);
		name = keyname(key);
		if (name)
			sprintf(temp, "key %s(0x%x)", name, key);
		else
			sprintf(temp, "key %s(0x%x)", mykeyname(key), key);
		mesg = temp;
		addCDKSwindow(swindow, mesg, BOTTOM);
		drawCDKSwindow(swindow, TRUE);
		/* exit if ESC */
		if (key == 033)
			break;
	}

	destroyCDKSwindow(swindow);
	destroyCDKScreen(screen);

	return 0;
}

module_t module_info_sym = {
	.version = 0,
	.id = "hardware",
	.name = "keyboard",
	.handler = keyboard_handler,
};
