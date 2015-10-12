#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <msg.h>
#include <list.h>
#include <loop.h>
#include <socket.h>

typedef struct _client {

    int sock;

    char name[32];

    int msg_queue;

    poll_t *poll;

    int (*onKeyEvent)(struct _client *cli, struct input_event ev);
    int (*onMotion)(struct _client *cli, struct input_event ev);
    int (*onStart)(struct _client *cli);
    int (*onStop)(struct _client *cli);
    int (*onPause)(struct _client *cli);
    int (*onGenericEvent)(struct _client *cli, msg_t msg);

} client_t;

#define cli_dbg(fmt, arg...) printf("/CLI/ " fmt, ##arg)

#endif
