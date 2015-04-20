#ifndef __EVENT_H__
#define __EVENT_H__

#include "win.h"

#define EVENT_UNKNOWN	(KEY_MAX + 1)
#define EVENT_UEVENT	(KEY_MAX + 2)
#define EVENT_EXIT		(KEY_MAX + 3)
#define EVENT_FOCUS		(KEY_MAX + 4)

extern int is_focus(modwin_t *modwin);
extern int get_event(modwin_t *modwin);
extern int cancel_subwin(modwin_t *modwin);
extern int enter_subwin(modwin_t *modwin);
extern int exit_subwin(modwin_t *modwin);
extern int event_init(mainwin_t *mainiwin);

#endif
