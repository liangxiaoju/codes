#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>

#define SRV_SOCKET_PATH "/dev/socket/adbd"

void _dumpline(long addr, char *buf, int len)
{
    int i, pos;
    char line[80+1];

    // Address field
    pos = sprintf(line, "%08lx ", addr);

    // Hex content
    for (i = 0; i < 16; ++i) {
        if (i % (16/2) == 0) {
            line[pos++] = ' '; // Insert a space
        }

        if (i < len) {
            pos += sprintf(&line[pos], "%02x ", buf[i]);
        } else {
            pos += sprintf(&line[pos], "   ");
        }
    }
    pos += sprintf(&line[pos], " |");

    // Printable content
    for (i = 0; i < len; ++i) {
        line[pos++] = isprint(buf[i]) ? buf[i] : '.';
    }

    sprintf(&line[pos], "|\n");
    printf("%s", line);
}

void hexdump(char *buf, int len)
{
    int i;
    for (i = 0; i < (len/16); ++i) {
        _dumpline(16*i, &buf[16*i], 16);
    }
    // Dump remaining which len is not 16
    if (len % 16 != 0) {
        _dumpline(16*i, &buf[16*i], len % 16);
    }
}

int cli_socket_init()
{
    struct sockaddr_un srv_addr;
    int cli_sockfd = -1;
    int rc;

    cli_sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (cli_sockfd < 0) {
        perror("socket");
        return -1;
    }
	fprintf(stdout, "CREATE SOCKET [%d]\n", cli_sockfd);

    memset(&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sun_family = AF_UNIX;
    strcpy(srv_addr.sun_path, SRV_SOCKET_PATH);

    rc = connect(cli_sockfd, (struct sockaddr *)&srv_addr, sizeof(srv_addr));
    if (rc < 0) {
        close(cli_sockfd);
        perror("connect");
        return -1;
    }

    rc = fcntl(cli_sockfd, F_SETFL, O_NONBLOCK);
    if (rc < 0) {
        perror("fcntl(O_NONBLOCK)");
        return -1;
    }

	fprintf(stdout, "CONNECT TO '%s' SUCCESS.\n", SRV_SOCKET_PATH);

    return cli_sockfd;
}

int exec(const char *cmd)
{
	int pid;
	int status;

	pid = fork();
	if (pid < 0) {
		perror("fork");
		return -1;
	}

	if (pid == 0) {
		if (execlp("sh", "sh", "-c", cmd, NULL)) {
			perror("execlp");
			exit(-1);
		}
	}

	if (waitpid(pid, &status, 0) < 0) {
		perror("waitpid");
		return -1;
	}

	if (WIFEXITED(status)) {
		;
	} else {
		fprintf(stdout, "Failed to exec '%s'\n", cmd);
		return -1;
	}


	return 0;
}

void start_adbd(void)
{
	int ret;

	fprintf(stdout, "start adbd\n");
	ret = exec("setprop sys.usb.config mtp,adb");
	if (ret < 0)
		perror("system");

	sleep(1);
}

void stop_adbd(void)
{
	int ret;

	fprintf(stdout, "stop adbd\n");
	ret = exec("setprop sys.usb.config mtp");
	if (ret < 0)
		perror("system");

	sleep(1);
}

int main(int argc, char *argv[])
{
	char buf[4096];
	int len;
	int fd = -1;
	int nready = -1;

	setlinebuf(stdout);
	fprintf(stdout, "start...\n");

	for (;;) {
		struct timeval timeout = {5, 0};
		fd_set readfds;

		start_adbd();

		if (nready <= 0) {
			fprintf(stdout, "try open sock ...\n");
			if (fd > 0)
				close(fd);
			fd = cli_socket_init();
			if (fd <= 0) {
				sleep(1);
				continue;
			}
		}

		FD_ZERO(&readfds);
		FD_SET(fd, &readfds);
		int nready = select(fd+1, &readfds, NULL, NULL, &timeout);
		if (nready > 0) {
			if (FD_ISSET(fd, &readfds)) {
				len = recv(fd, buf, sizeof(buf), 0);
				hexdump(buf, len);

				if (buf[0] == 'P' && buf[1] == 'K') {
					snprintf(buf, sizeof(buf), "OK");
					send(fd, buf, strlen(buf)+1, 0);
					break;
				}
			}
		}

		stop_adbd();
	}

	fprintf(stdout, "done.\n");

	return 0;
}
