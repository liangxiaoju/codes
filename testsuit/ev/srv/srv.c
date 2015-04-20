/*
 * a implementation of server
 */

#include <stdio.h>

#include <server.h>

int srv_cli_connect(server_t *srv, int sock)
{
    msg_t msg = { .mtype = MSG_TYPE_SYSTEM, .system_event.notice = NOTICE_STARTUP };
    send_msg(sock, msg);
}

int srv_cli_request(server_t *srv, int sock, msg_t msg)
{
    printf("%s type:%ld text:%s\n", __func__, msg.mtype, msg.mtext);
}

int srv_input_event(server_t *srv, struct input_event event)
{
    msg_t msg = { .mtype = MSG_TYPE_INPUT_EVENT, .input_event = event };
    msgsnd(srv->msg_queue, &msg, sizeof(event), 0);
}

static server_t srv = {
    .onCliConnect = srv_cli_connect,
    .onCliRequest = srv_cli_request,
    .onInputEvent = srv_input_event,
};

int main(int argc, char *argv[])
{
    return SRV_RUN(&srv);
}
