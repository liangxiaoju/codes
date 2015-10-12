#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <msg.h>

#define SRV_SOCKET_PATH "/tmp/srv.sock"

extern int srv_socket_init();
extern int srv_socket_accept(int sock);
extern int cli_socket_init();
extern int recv_msg(int sock, msg_t *msg);
extern int send_msg(int sock, msg_t msg);

#endif
