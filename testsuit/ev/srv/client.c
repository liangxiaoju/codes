#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "client.h"

struct cli_rq_priv {
    server_t *srv;
    poll_t *poll;
    int sock;
};

static struct cli_rq_priv *cli_rq;

int srv_client_callback(int sock, unsigned int event, void *data);

void srv_add_client(struct cli_rq_priv *cli_rq, int cli_sockfd)
{
    server_t *srv = cli_rq->srv;
    struct client *cli;

    poll_add(cli_rq->poll, cli_sockfd, srv_client_callback, cli_rq);

    cli =  malloc(sizeof(*cli));
    memset(cli, 0, sizeof(*cli));
    cli->sock = cli_sockfd;

    list_add(&cli->node, &srv->clients);
}

void srv_del_client(struct cli_rq_priv *cli_rq, int cli_sockfd)
{
    server_t *srv = cli_rq->srv;
    struct client *cli, *cli_tmp;

    list_for_each_entry_safe(cli, cli_tmp, &srv->clients, node) {
        if (cli->sock == cli_sockfd) {
            list_del(&cli->node);
            free(cli);
        }
    }

    poll_del(cli_rq->poll, cli_sockfd);
}

int srv_client_callback(int sock, unsigned int event, void *data)
{
    struct cli_rq_priv *cli_rq = data;
    server_t *srv = cli_rq->srv;
    msg_t msg;

    if (recv_msg(sock, &msg) <= 0) {
        srv_dbg("[disconnect] %d\n", sock);
        srv_del_client(cli_rq, sock);
    } else {
        srv_dbg("[msg] type:%ld text:%s\n", msg.mtype, msg.mtext);
        if (srv->onCliRequest)
            srv->onCliRequest(srv, sock, msg);
    }

    return 0;
}

int srv_connect_callback(int fd, unsigned int event, void *data)
{
    struct cli_rq_priv *cli_rq = data;
    server_t *srv = cli_rq->srv;
    int new_sock;

    new_sock = srv_socket_accept(cli_rq->sock);

    srv_dbg("[connect] %d\n", new_sock);

    srv_add_client(cli_rq, new_sock);

    if (srv->onCliConnect)
        srv->onCliConnect(srv, new_sock);

    return 0;
}

int _cli_rq_init(server_t *srv)
{
    cli_rq = malloc(sizeof(*cli_rq));

    cli_rq->srv = srv;

    cli_rq->poll = poll_create(10);

    cli_rq->sock = srv_socket_init();
    if (cli_rq->sock < 0)
        return -1;

    poll_add(cli_rq->poll, cli_rq->sock, srv_connect_callback, cli_rq);
}

void *cli_request_handle(void *arg)
{
    server_t *srv = arg;

    _cli_rq_init(srv);

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    srv_dbg("start cli-req loop ...\n");

    do {
        poll_run(cli_rq->poll, -1);
    } while (1);

    pthread_exit(NULL);
}

void cli_request_exit(void)
{
    close(cli_rq->sock);
}

int cli_request_run(server_t *srv)
{
    atexit(cli_request_exit);
    return pthread_create(&srv->client_thread, NULL, cli_request_handle, srv);
}
