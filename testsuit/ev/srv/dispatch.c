#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "dispatch.h"
#include "client.h"

void *msg_dispatch(void *arg)
{
    server_t *srv = arg;
    struct client *cli, *tmp;
    msg_t msg;

    srv_dbg("[dispatcher] start.\n");

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    do {
        if (msgrcv(srv->msg_queue, &msg, MSG_DATA_SIZE, 0, 0) < 0) {
            perror("msgrcv");
        } else {
            list_for_each_entry_safe(cli, tmp, &srv->clients, node) {
                send_msg(cli->sock, msg);
            }
        }
    } while (1);

    srv_dbg("[dispatcher] exit.\n");

    pthread_exit(NULL);
}

int msg_dispatch_run(server_t *srv)
{
    return pthread_create(&srv->dispatch_thread, NULL, msg_dispatch, srv);
}
