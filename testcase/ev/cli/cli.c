/*
 * an example of client
 */

#include <stdio.h>

#include "client.h"

int cli_onKeyEvent(client_t *cli, struct input_event ev)
{
    printf("key: code=0x%x value=0x%x\n", ev.code, ev.value);
}

int cli_onGenericEvent()
{
}

int cli_onStart(client_t *cli)
{
    printf("%s\n", __func__);
}

static client_t cli = {
    .name = "cli-1",
    .onStart = cli_onStart,
    .onKeyEvent = cli_onKeyEvent,
    .onGenericEvent = cli_onGenericEvent,
};

int main(int argc, char *argv[])
{
    if (CLI_INIT(&cli) < 0)
        return -1;

    CLI_LOOP(&cli);

    CLI_EXIT(&cli);

    return 0;
}
