#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>

#include <loop.h>

struct request {
    int fd;
    unsigned int event;
    void *arg;
    req_callback_t cb;
    struct list_head node;
};

static struct request *req_alloc()
{
    struct request *req;

    req = malloc(sizeof(*req));
    memset(req, 0, sizeof(*req));
    INIT_LIST_HEAD(&req->node);

    return req;
}

static void req_free(struct request *req)
{
    free(req);
}

poll_t *poll_create(int n)
{
    poll_t *p;
    struct request *req;
    int i;

    p = malloc(sizeof(poll_t));
    if (p == NULL)
        return NULL;
    memset(p, 0, sizeof(poll_t));

    p->pollfd = epoll_create(10);
    if (p->pollfd < 0) {
        perror("epoll_create");
        return NULL;
    }

    INIT_LIST_HEAD(&p->reqs);

    for (i = 0; i < n; i++) {
        req = req_alloc();
        list_add_tail(&req->node, &p->reqs);
    }

    return p;
}

void poll_close(poll_t *p)
{
    struct request *req, *tmp;

    list_for_each_entry_safe(req, tmp, &p->reqs, node) {
        list_del(&req->node);
        req_free(req);
    }

    close(p->pollfd);
    free(p);
}

int poll_add(poll_t *p, int fd, req_callback_t cb, void *arg)
{
    struct epoll_event ev;
    struct request *req;
    int rc, found = 0;

    memset(&ev, 0, sizeof(ev));
    ev.events = EPOLLIN;
    ev.data.fd = fd;
    rc = epoll_ctl(p->pollfd, EPOLL_CTL_ADD, fd, &ev);
    if (rc < 0) {
        perror("epoll_ctl(EPOLL_CTL_ADD)");
        return -1;
    }

    list_for_each_entry(req, &p->reqs, node) {
        if ((req->fd == 0) || (req->fd == fd)) {
            found = 1;
            break;
        }
    }

    if (!found) {
        req = req_alloc();
        list_add_tail(&req->node, &p->reqs);
    }

    req->fd = fd;
    req->cb = cb;
    req->arg = arg;

    return 0;
}

int poll_del(poll_t *p, int fd)
{
    struct request *req;
    int rc;

    list_for_each_entry(req, &p->reqs, node) {
        if (req->fd == fd) {
            /* set to 0 for reuse*/
            req->fd = 0;
            break;
        }
    }

    rc = epoll_ctl(p->pollfd, EPOLL_CTL_DEL, fd, NULL);
    if (rc < 0) {
        perror("epoll_ctl(EPOLL_CTL_DEL)");
        return -1;
    }

    return 0;
}

#define MAX_EVENTS 10
int poll_run(poll_t *p, int timeout)
{
    struct epoll_event events[MAX_EVENTS];
    struct request *req, *tmp;
    int nfds, n, fd;
    unsigned int event;

    nfds = epoll_wait(p->pollfd, events, MAX_EVENTS, timeout);
    if (nfds < 0) {
        perror("epoll_wait");
        return -1;
    }

    for (n = 0; n < nfds; n++) {
        fd = events[n].data.fd;
        event = events[n].events;
        list_for_each_entry_safe(req, tmp, &p->reqs, node) {
            if (req->fd == fd) {
                req->cb(fd, event, req->arg);
                break;
            }
        }
    }

    return nfds;
}
