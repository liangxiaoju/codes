#ifndef __SOCKETCLIENT_H__
#define __SOCKETCLIENT_H__

#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/time.h>

#include <iostream>
#include <sstream>
#include <thread>

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

class SocketClient
{
public:
	virtual void onConnect()
	{
		std::cout << "onConnect" << std::endl;
	}
	virtual void onClose()
	{
		std::cout << "onClose" << std::endl;
	}
	virtual void onError(std::string err)
	{
		std::cout << "onError: " << err << std::endl;
	}
	virtual void onMessage(std::string msg)
	{
		std::cout << "onMessage: " << msg << std::endl;
	}

	SocketClient(std::string host, int port)
	: _host(host)
	, _port(port)
	, _stop(false)
	{
	}

	int connect()
	{
		_sock = socket(AF_INET, SOCK_STREAM, 0);
		if (_sock < 0)
			return -1;

		const int on = 1;
		setsockopt(_sock, SOL_SOCKET, SO_REUSEADDR,
				(const void*)&on, sizeof(on));
#ifdef SO_REUSEPORT
		setsockopt(_sock, SOL_SOCKET, SO_REUSEPORT,
				(const void*)&on, sizeof(on));
#endif

#ifdef SO_NOSIGPIPE
		setsockopt(_sock, SOL_SOCKET, SO_NOSIGPIPE,
				(const void *)&on, sizeof(on));
#endif

		struct sockaddr_in addr;
		memset(&addr, 0, sizeof(struct sockaddr_in));
		addr.sin_family = AF_INET;
		if (inet_aton(_host.c_str(), &addr.sin_addr) == 0) {
			close(_sock);
			onError("inet_aton");
			return -1;
		}
		addr.sin_port = htons(_port);
		socklen_t len = sizeof(struct sockaddr_in);

		if (::connect(_sock, (const struct sockaddr*)&addr, len) < 0) {
			close(_sock);
			onError("connect");
			return -1;
		}

		onConnect();

		pipe(_pipe);

		_thread = std::thread([this] { loop(); });

		return 0;
	}

	void disconnect()
	{
		_stop = true;
		write(_pipe[1], "W", 1);
		_thread.join();
		close(_sock);
		close(_pipe[0]);
		close(_pipe[1]);
	}

	int send(std::string msg)
	{
		return ::send(_sock, msg.c_str(), msg.size(), MSG_NOSIGNAL);
	}

private:
	void loop()
	{
		for (;;) {
			fd_set read_set;
			FD_ZERO(&read_set);
			FD_SET(_sock, &read_set);
			FD_SET(_pipe[0], &read_set);

			int nready = select(std::max(_sock, _pipe[0])+1, &read_set, NULL, NULL, NULL);
			if (_stop)
				return;

			if (nready == -1) {
				onError("select");
				return;
			} else if (nready == 0) {
				//std::cout << "Timeout" << std::endl;
			} else {
				if (FD_ISSET(_sock, &read_set)) {
					char buf[4096];
					memset(buf, 0, sizeof(buf));
					int len = recv(_sock, buf, sizeof(buf), 0);
					if (len <= 0) {
						onClose();
						close(_sock);
					} else {
						std::string msg(buf, len);
						onMessage(msg);
					}
				}
			}
		}
	}

	std::thread _thread;
	std::string _host;
	int _port;
	int _sock;
	bool _stop;
	int _pipe[2];
};

#endif
