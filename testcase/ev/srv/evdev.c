#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <dirent.h>
#include <sys/inotify.h>

#include "evdev.h"

#define DEVICE_PATH "/dev/input"

struct input_device {
    int fd;
    char devname[1024];
    struct list_head node;
};

struct evdev_priv {
    poll_t *poll;
    server_t *srv;
    struct list_head devs;
};

static struct evdev_priv *evdev;

int _evdev_read_callback(int fd, unsigned int ev, void *data)
{
    struct evdev_priv *evdev = data;
    server_t *srv = evdev->srv;
    int rc;

    struct input_event event;
    do {
        rc = read(fd, &event, sizeof(event));
        if (rc <= 0)
            break;
        if (srv->onInputEvent)
            srv->onInputEvent(srv, event);
    } while (1);
}

void _add_device(struct evdev_priv *evdev, char *devname)
{
    struct input_device *dev;
    int fd, i = 3;

    while ((fd = open(devname, O_RDONLY)) < 0) {
        if (errno == EACCES) {

            if (--i <= 0)
                return;

            /* waiting for udev to modify dev mode */
            usleep(100*1000);
            continue;

        } else {
            perror("open");
            return ;
        }
    }

    fcntl(fd, F_SETFL, O_NONBLOCK);

    dev = malloc(sizeof(*dev));
    if (!dev) {
        perror("malloc");
        return ;
    }
    dev->fd = fd;
    strcpy(dev->devname, devname);

    list_add_tail(&dev->node, &evdev->devs);

    poll_add(evdev->poll, fd, _evdev_read_callback, evdev);

    srv_dbg("add device: %s\n", devname);
}

void _del_device(struct evdev_priv *evdev, char *devname)
{
    struct input_device *dev, *tmp;

    list_for_each_entry_safe(dev, tmp, &evdev->devs, node) {
        if (strcmp(dev->devname, devname) == 0) {
            poll_del(evdev->poll, dev->fd);
            list_del(&dev->node);
            close(dev->fd);
            free(dev);
        }
    }
    srv_dbg("del device: %s\n", devname);
}

void _evdev_scandev(struct evdev_priv *evdev)
{
    char devname[1024];
    DIR *dir;
    struct dirent *de;

    srv_dbg("scan devices...\n");
    dir = opendir(DEVICE_PATH);
    if (dir == NULL) {
        perror("opendir");
        return ;
    }

    while ((de = readdir(dir))) {
        if (de->d_type != DT_CHR)
            continue;
        snprintf(devname, sizeof(devname), "%s/%s", DEVICE_PATH, de->d_name);
        _add_device(evdev, devname);
    }
    closedir(dir);
}

int _evdev_inotify_callback(int fd, unsigned int ev, void *data)
{
    char devname[1024], buf[512];
    struct evdev_priv *evdev = data;
    struct inotify_event *event;
    int event_len, len;

    len = read(fd, buf, sizeof(buf));
    if (len < sizeof(struct inotify_event))
        return -1;

    event = (struct inotify_event *)buf;
    while (len >= sizeof(struct inotify_event)) {

        if (event->len) {
            snprintf(devname, sizeof(devname), "%s/%s", DEVICE_PATH, event->name);
            printf("%s\n", devname);
            if (event->mask & IN_CREATE) {
                _add_device(evdev, devname);
            } else if (event->mask & IN_DELETE) {
                _del_device(evdev, devname);
            }
        }

        event_len = sizeof(struct inotify_event) + event->len;
        event = (struct inotify_event *)((void *)event + event_len);
        len -= event_len;
    }
}

int _evdev_init(server_t *srv)
{
    int inotify_fd;

    evdev = malloc(sizeof(*evdev));

    evdev->srv = srv;
    evdev->poll = poll_create(5);
    if (evdev->poll == NULL)
        return -1;

    INIT_LIST_HEAD(&evdev->devs);

    _evdev_scandev(evdev);

    inotify_fd = inotify_init();
    if (inotify_fd < 0) {
        perror("inotify_init");
        return -1;
    }

    if (inotify_add_watch(inotify_fd, DEVICE_PATH, IN_DELETE | IN_CREATE) < 0) {
        perror("inotify_add_watch");
        return -1;
    }

    poll_add(evdev->poll, inotify_fd, _evdev_inotify_callback, evdev);
}

void *evdev_handle(void *arg)
{
    server_t *srv = arg;

    _evdev_init(srv);

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    srv_dbg("evdev loop\n");
    do {
        poll_run(evdev->poll, -1);
    } while (1);

    pthread_exit(NULL);
}

void evdev_exit(void)
{
    poll_close(evdev->poll);
}

int evdev_run(server_t *srv)
{
    atexit(evdev_exit);
    return pthread_create(&srv->evdev_thread, NULL, evdev_handle, srv);
}
