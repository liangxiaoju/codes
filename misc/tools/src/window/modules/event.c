#include "module.h"
#include "msg.h"

int event_handler(WINDOW *win, void *ptr)
{
	CDKSCREEN *screen;

	screen = initCDKScreen(win);

	destroyCDKScreen(screen);

	return 0;
}

module_t module_info_sym = {
	.version = 0,
	.id = "hardware",
	.name = "event",
	.handler = event_handler,
};
