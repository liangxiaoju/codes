#ifndef __WIN_H__
#define __WIN_H__

#include <curses.h>
#include <cdk.h>
#include "module.h"

#define MAX_MODULE 16

#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif

#define container_of(ptr, type, member)						\
({		      												\
	const typeof(((type *)0)->member)*__mptr = (ptr);    	\
	(type *)((char *)__mptr - offsetof(type, member));		\
})

struct boxwin;
struct module;

typedef struct {
	WINDOW *win;
} barwin_t;

typedef struct {
	WINDOW *win;
} modwin_t;

typedef struct {
	WINDOW *win;
} sbarwin_t;

typedef struct {
	WINDOW *win;

	sbarwin_t sbarwin;

	modwin_t modwin;

	struct module *module;

	pthread_t tid;

	struct boxwin *boxwin;

	int id;

	int group;

	int assigned;

	int uevent_pending;
	int cancel_pending;
	int focus_changed;

} subwin_t;

typedef struct boxwin {
	WINDOW *win;

	subwin_t subwin[MAX_MODULE];
	int nsubs;

	int curr;

	int groups;
	int igroup;

} boxwin_t;

typedef struct mainwin {
	WINDOW *win;

	barwin_t bar;
	boxwin_t box;

} mainwin_t;

extern mainwin_t g_mainwin;

#define to_subwin(win) container_of(win, subwin_t, modwin)
#define to_boxwin(win) (to_subwin(win)->boxwin)
#define to_mainwin(win) container_of(to_boxwin(win), mainwin_t, box)
#define to_id(win) (to_subwin(win)->id)

#endif
