#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include <server.h>

#include "client.h"
#include "evdev.h"
#include "dispatch.h"

int SRV_RUN(server_t *srv)
{
    INIT_LIST_HEAD(&srv->clients);

    key_t key = ftok("/tmp/msg", 1);
    srv->msg_queue = msgget(key, IPC_CREAT | 0644);
    if (srv->msg_queue < 0) {
        perror("msgget");
        return -1;
    }

    cli_request_run(srv);
    evdev_run(srv);
    msg_dispatch_run(srv);

    do {
        sleep(10);
    } while (1);

    return 0;
}
