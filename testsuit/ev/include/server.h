#ifndef __SERVER_H__
#define __SERVER_H__

#include <linux/input.h>
#include <pthread.h>

#include <msg.h>
#include <list.h>
#include <loop.h>
#include <socket.h>

#define SRV_NAME "server"

typedef struct _server {

    int sock;

    int msg_queue;

    struct list_head clients;

    //poll_t *poll;

    pthread_t client_thread;
    pthread_t evdev_thread;
    pthread_t dispatch_thread;

    int (*onCliRequest)(struct _server *srv, int sock, msg_t msg);
    int (*onInputEvent)(struct _server *srv, struct input_event event);
    int (*onCliConnect)(struct _server *srv, int sock);

} server_t;

#define srv_dbg(fmt, arg...) printf("/SRV/ " fmt, ##arg)

extern int SRV_RUN(server_t *srv);

#endif
