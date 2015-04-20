#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>

#include <socket.h>

int srv_socket_init()
{
    struct sockaddr_un srv_addr;
    int srv_sockfd = -1;
    int rc;

    srv_sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (srv_sockfd < 0) {
        perror("socket");
        return -1;
    }

    memset(&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sun_family = AF_UNIX;
    strcpy(srv_addr.sun_path, SRV_SOCKET_PATH);

    unlink(SRV_SOCKET_PATH);
    rc = bind(srv_sockfd, (struct sockaddr *)&srv_addr, sizeof(srv_addr));
    if (rc < 0) {
        close(srv_sockfd);
        perror("bind");
        return -1;
    }

    rc = listen(srv_sockfd, 10);
    if (rc < 0) {
        close(srv_sockfd);
        unlink(SRV_SOCKET_PATH);
        perror("listen");
        return -1;
    }

    return srv_sockfd;
}

int srv_socket_accept(int sock)
{
    struct sockaddr_un cli_addr;
    socklen_t len = sizeof(cli_addr);
    int new_sock;

    new_sock = accept(sock, (struct sockaddr *)&cli_addr, &len);
    if (new_sock < 0) {
        perror("accept");
        return -1;
    }

    fcntl(sock, F_SETFL, O_NONBLOCK);

    return new_sock;
}

int cli_socket_init()
{
    struct sockaddr_un srv_addr;
    int cli_sockfd = -1;
    int len = MSG_SIZE;
    int rc;

    cli_sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (cli_sockfd < 0) {
        perror("socket");
        return -1;
    }

    memset(&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sun_family = AF_UNIX;
    strcpy(srv_addr.sun_path, SRV_SOCKET_PATH);

    rc = connect(cli_sockfd, (struct sockaddr *)&srv_addr, sizeof(srv_addr));
    if (rc < 0) {
        close(cli_sockfd);
        perror("connect");
        return -1;
    }

    rc = setsockopt(cli_sockfd, SOL_SOCKET, SO_RCVLOWAT, &len, sizeof(len));
    if (rc < 0) {
        perror("setsockopt(SO_RCVLOWAT)");
        return -1;
    }

    rc = fcntl(cli_sockfd, F_SETFL, O_NONBLOCK);
    if (rc < 0) {
        perror("fcntl(O_NONBLOCK)");
        return -1;
    }

    return cli_sockfd;
}

int recv_msg(int sock, msg_t *msg)
{
    int rc;

    rc = recv(sock, msg, MSG_SIZE, 0);
    if (rc < 0) {
        perror("recv");
        return -1;
    }

    return rc;
}

int send_msg(int sock, msg_t msg)
{
    int rc;

    rc = send(sock, &msg, MSG_SIZE, 0);
    if (rc < 0) {
        perror("send");
        return -1;
    }

    return rc;
}
