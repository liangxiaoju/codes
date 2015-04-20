#include <string.h>
#include <poll.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <linux/netlink.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>


static int uevent_init()
{
	struct sockaddr_nl addr;
	int sz = 64 * 1024;
        int on = 1;
	int fd;

	memset(&addr, 0, sizeof(addr));
	addr.nl_family = AF_NETLINK;
	addr.nl_pid = getpid();
	addr.nl_groups = 0xffffffff;

	fd = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
	if (fd < 0) {
		printf("Failed to create socket !\n");
		return -1;
	}
	setsockopt(fd, SOL_SOCKET, SO_RCVBUFFORCE, &sz, sizeof(sz));
	setsockopt(fd, SOL_SOCKET, SO_PASSCRED, &on, sizeof(on));


	if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		printf("Failed to bind socket !\n");
		close(fd);
		return -1;
	}
	return fd;
}


int main()
{
	int fd, n;
	char buf[1024] = { 0 };

	fd = uevent_init();
	if(fd < 0) {
		printf("Failed to exec uevent_init !\n");
		return -1;
	}

	while ((n = recv(fd, buf, sizeof(buf), 0)) > 0) {
		printf("socket (%d): %s\n", n, buf);
		memset(buf, 0, sizeof(buf));
	}

	return 0;
}

