#include <stdio.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <sys/reboot.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/sendfile.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <linux/netlink.h>
#include <linux/fb.h>
#include <time.h>

#define UEVENT_MSG_LEN 1024

#define ERROR(fmt, arg...) fprintf(stderr, fmt, ##arg)
#define INFO(fmt, arg...) fprintf(stdout, fmt, ##arg)
#define NOTICE(fmt, arg...) fprintf(stdout, fmt, ##arg)

typedef struct {
    int uevent_fd;
    char *filter;
    char *filename;
} event_t;

struct uevent {
    const char *res;
};

int event_init(event_t *event)
{
    struct sockaddr_nl addr;
    int buf_sz = 64 * 1024;
    int on = 1;
    int s;

    memset(&addr, 0, sizeof(addr));
    addr.nl_family = AF_NETLINK;
    addr.nl_pid = getpid();
    addr.nl_groups = 0xffffffff;

    s = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
    if (s < 0)
        return -1;

    setsockopt(s, SOL_SOCKET, SO_RCVBUFFORCE, &buf_sz, sizeof(buf_sz));
    setsockopt(s, SOL_SOCKET, SO_PASSCRED, &on, sizeof(on));

    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(s);
        return -1;
    }

//    fcntl(s, F_SETFL, O_NONBLOCK);

    event->uevent_fd = s;

    INFO("add kernel uevent\n");

    return s;
}

ssize_t uevent_kernel_multicast_recv(int socket, void *buffer, size_t length)
{
    int n;

    n = recv(socket, buffer, length, 0);
    if (n < 0)
        return 0;

    for (int i = 0; i < n; i++) {
        if (((char*)buffer)[i] == '\0')
            ((char*)buffer)[i] = ' ';
    }
    ((char*)buffer)[n] = '\0';

    INFO("socket (%d): %s\n", n, (char *)buffer);

    return n;
}

int parse_uevent(const char *msg, struct uevent *uevent)
{
    uevent->res = msg;
    return 0;
}

void child(int argc, const char *argv[])
{
    char *argv_child[argc + 1];

    memcpy(argv_child, argv, argc * sizeof(char *));
    argv_child[argc] = NULL;

    if (execv(argv_child[0], argv_child)) {
        ERROR("executing %s failed: %s\n",
                argv_child[0], strerror(errno));
        exit(-1);
    }
}

int run(char *fn, const char *args)
{
    pid_t pid;

    pid = fork();
    if (pid < 0) {
        return -1;
    } else if (pid == 0) {
        int argc = 2;
        const char *argv[] = { strdup(fn), strdup(args), NULL };
        child(argc, argv);
    } else {
        int status;
        waitpid(pid, &status, 0);
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            ERROR("Error exec %s !\n(Status %d)\n", fn,
                    WEXITSTATUS(status));
            return -1;
        }
    }
    return 0;
}

int process_uevent(event_t *event, struct uevent *uevent)
{
    int ret = 0;

    if (strstr(uevent->res, event->filter)) {
        ret = run(event->filename, uevent->res);
    }

    return ret;
}

int handle_uevent(event_t *event)
{
    char msg[UEVENT_MSG_LEN + 2];
    int n, ret = 0;

    while (true) {
        struct uevent uevent;

        n = uevent_kernel_multicast_recv(event->uevent_fd, msg, UEVENT_MSG_LEN);
        if (n <= 0)
            break;
        if (n >= UEVENT_MSG_LEN)
            continue;

        msg[n] = '\0';
        msg[n+1] = '\0';

        parse_uevent(msg, &uevent);
        ret = process_uevent(event, &uevent);
    }

    return ret;
}

void event_loop(event_t *event)
{
    handle_uevent(event);
}

int main(int argc, char *argv[])
{
    event_t event;

    if (argc != 3) {
        ERROR("Illegal param!\n");
        return -1;
    }

    memset(&event, 0, sizeof(event_t));

    event.filter=strdup(argv[1]);
    event.filename=strdup(argv[2]);

    event_init(&event);

    event_loop(&event);

    return 0;
}

