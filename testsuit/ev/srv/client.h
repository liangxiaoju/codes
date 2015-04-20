#ifndef __CLIENT_C__
#define __CLIENT_C__

#include <list.h>
#include <loop.h>
#include <socket.h>
#include <server.h>

struct client {

    int sock;

    struct list_head node;

};

extern int cli_request_run(server_t *srv);

#endif
