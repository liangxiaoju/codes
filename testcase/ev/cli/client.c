#include <stdio.h>
#include <stdlib.h>

#include "client.h"

int handle_input_event(client_t *cli, struct input_event ev)
{
    switch (ev.type) {
    case EV_KEY:
        if (cli->onKeyEvent)
            cli->onKeyEvent(cli, ev);
        break;
    case EV_REL:
    case EV_ABS:
        if (cli->onMotion)
            cli->onMotion(cli, ev);
        break;
    }
}

int handle_system_msg(client_t *cli, struct system_event ev)
{
    switch (ev.notice) {
    case NOTICE_STARTUP:
        if (cli->onStart)
            cli->onStart(cli);
        break;
    case NOTICE_STOP:
        if (cli->onStop)
            cli->onStop(cli);
        break;
    case NOTICE_PAUSE:
        if (cli->onPause)
            cli->onPause(cli);
        break;
    }
}

int _cli_server_callback(int sock, unsigned int event, void *data)
{
#if 1
    client_t *cli = data;
    msg_t msg;

    if (recv_msg(sock, &msg) == 0) {
        cli_dbg("[lost]\n");
        poll_del(cli->poll, sock);
    } else {
        //cli_dbg("[msg] type:%ld mtext:%s\n", msg.mtype, msg.mtext);

        //parse_msg(msg);

        switch (msg.mtype) {
        case MSG_TYPE_INPUT_EVENT:
            handle_input_event(cli, msg.input_event);
            break;
        case MSG_TYPE_SYSTEM:
            handle_system_msg(cli, msg.system_event);
            break;
        default:
            cli->onGenericEvent(cli, msg);
        }
    }
#endif
}

int CLI_INIT(client_t *cli)
{
    cli->sock = cli_socket_init();
    if (cli->sock < 0)
        return -1;

    cli->poll = poll_create(10);
    poll_add(cli->poll, cli->sock, _cli_server_callback, cli);

    return 0;
}

int CLI_LOOP(client_t *cli)
{
    do {
        poll_run(cli->poll, -1);
    } while (1);
}

void CLI_EXIT(client_t *cli)
{
    poll_close(cli->poll);
    if (cli->sock > 0)
        close(cli->sock);
}
