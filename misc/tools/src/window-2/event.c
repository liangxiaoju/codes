#include <linux/netlink.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>
#include "win.h"
#include "thread.h"
#include "event.h"

static pthread_mutex_t event_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t event_cond = PTHREAD_COND_INITIALIZER;
static int uevent = 0;

int uevent_init(void)
{
	struct sockaddr_nl addr;
	int sz = 64 * 1024;
	int fd;

	memset(&addr,0,sizeof(addr));
	addr.nl_family = AF_NETLINK;
	addr.nl_pid = getpid();
	addr.nl_groups = 0xffffffff;

	fd = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
	if(fd<0) {
		fprintf(stderr, "Failed to create socket !\n");
		return -1;
	}
	setsockopt(fd, SOL_SOCKET, SO_RCVBUFFORCE, &sz, sizeof(sz));

	if(bind(fd, (struct sockaddr *)&addr, sizeof(addr))<0) {
		fprintf(stderr, "Failed to bind socket !\n");
		close(fd);
		return -1;
	}
	return fd;
}

void *uevent_monitor(void *arg)
{
	char buf[256];
	int ret = 0, nbytes, i;
	int fd;
	mainwin_t *mainwin = arg;

	fd = uevent_init();
	if (fd < 0) {
		exit(-1);
	}

	while ((nbytes = recv(fd, buf, sizeof(buf), 0)) > 0) {
		pthread_mutex_lock(&event_lock);
		for (i = 0; i < mainwin->box.nsubs; i++)
			mainwin->box.subwin[i].uevent_pending = 1;
		pthread_cond_broadcast(&event_cond);
		pthread_mutex_unlock(&event_lock);
	}

	pthread_exit(NULL);
}

int event_init(mainwin_t *mainwin)
{
	pthread_t uevent_id;

	pthread_create(&uevent_id, NULL, uevent_monitor, mainwin);

	return 0;
}

int is_focus(modwin_t *modwin)
{
	mainwin_t *mainwin = &g_mainwin;
	int focus = 0;

	pthread_mutex_lock(&event_lock);
	if (!modwin) {
		if (mainwin->box.curr==-1)
			focus = 1;
	} else if (to_id(modwin) == mainwin->box.curr)
		focus = 1;
	else
		focus = 0;
	pthread_mutex_unlock(&event_lock);

	return focus;
}

int wait_all_event(modwin_t *modwin)
{
	int event;

	keypad(g_mainwin.bar.win, TRUE);
	nodelay(g_mainwin.bar.win, TRUE);
	for (;;) {

		window_lock();
		event = wgetch(g_mainwin.bar.win);
		window_unlock();

		if (event != ERR)
			break;

		pthread_mutex_trylock(&event_lock);
		if (modwin) {
			subwin_t *subwin = to_subwin(modwin);
			if (subwin->uevent_pending) {
				subwin->uevent_pending = 0;
				event = EVENT_UEVENT;
				pthread_mutex_unlock(&event_lock);
				break;
			}
		}
		pthread_mutex_unlock(&event_lock);

		usleep(1000);
	}

	return event;
}

int wait_uevent(modwin_t *modwin)
{
	int ret = EVENT_UNKNOWN;
	pthread_mutex_lock(&event_lock);
	if (modwin) {
		subwin_t *subwin = to_subwin(modwin);
		if (subwin->cancel_pending) {
			ret = EVENT_EXIT;
			subwin->cancel_pending = 0;
			goto out;
		} else if (subwin->focus_changed) {
			ret = EVENT_FOCUS;
			subwin->focus_changed = 0;
			goto out;
		} else if (subwin->uevent_pending) {
			ret = EVENT_UEVENT;
			subwin->uevent_pending = 0;
			goto out;
		}
	}
	pthread_cond_wait(&event_cond, &event_lock);
	if (modwin) {
		subwin_t *subwin = to_subwin(modwin);
		if (subwin->cancel_pending) {
			ret = EVENT_EXIT;
			subwin->cancel_pending = 0;
		} else if (subwin->focus_changed) {
			ret = EVENT_FOCUS;
			subwin->focus_changed = 0;
		} else if (subwin->uevent_pending) {
			ret = EVENT_UEVENT;
			subwin->uevent_pending = 0;
		}
	}
out:
	pthread_mutex_unlock(&event_lock);
	return ret;
}

int get_event(modwin_t *modwin)
{
	int ret;
	int focus = 0;

	for (;;) {

		focus = is_focus(modwin);

		if (focus)
			return wait_all_event(modwin);
		else {
			ret = wait_uevent(modwin);
			if ((ret != EVENT_FOCUS) && (ret != EVENT_UNKNOWN))
				return ret;
		}
	}

	return 0;
}

int cancel_subwin(modwin_t *modwin)
{
	pthread_mutex_lock(&event_lock);
	if (modwin) {
		subwin_t *subwin = to_subwin(modwin);
		subwin->cancel_pending = 1;
	}
	pthread_cond_broadcast(&event_cond);
	pthread_mutex_unlock(&event_lock);
	return 0;
}

int enter_subwin(modwin_t *modwin)
{
	mainwin_t *mainwin = to_mainwin(modwin);
	subwin_t *subwin = to_subwin(modwin);
	int id = to_id(modwin);

	pthread_mutex_lock(&event_lock);
	mainwin->box.curr = id;
	subwin->focus_changed = 1;
	pthread_cond_broadcast(&event_cond);
	pthread_mutex_unlock(&event_lock);

	return 0;
}

int exit_subwin(modwin_t *modwin)
{
	mainwin_t *mainwin = to_mainwin(modwin);
	subwin_t *subwin = to_subwin(modwin);
	int i;

	pthread_mutex_lock(&event_lock);
	mainwin->box.curr = -1;
	subwin->focus_changed = 1;
	pthread_cond_broadcast(&event_cond);
	pthread_mutex_unlock(&event_lock);

	return 0;
}

