#ifndef __LOOP_H__
#define __LOOP_H__

#include <list.h>

typedef struct {
    int pollfd;
    struct list_head reqs;
} poll_t;

typedef int (*req_callback_t)(int fd, unsigned int event, void *data);

extern poll_t *poll_create(int n);
extern void poll_close(poll_t *p);
extern int poll_add(poll_t *p, int fd, req_callback_t cb, void *arg);
extern int poll_del(poll_t *p, int fd);
extern int poll_run(poll_t *p, int timeout);

#endif
