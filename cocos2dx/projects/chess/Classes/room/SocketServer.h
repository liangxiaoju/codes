#ifndef __SOCKETSERVER_H__
#define __SOCKETSERVER_H__

#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <arpa/inet.h>

#include <iostream>
#include <sstream>
#include <algorithm>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

class SocketServer
{
public:
	virtual void onConnect(int fd)
	{
		std::cout << "onConnect: " << fd << std::endl;
	}

	virtual void onDisconnect(int fd)
	{
		std::cout << "onDisconnect: " << fd << std::endl;
	}

	virtual void onMessage(int fd, std::string msg)
	{
		std::cout << "onMessage: " << fd << " " << msg << std::endl;
	}

	virtual void onError(int fd, std::string err)
	{
	}

	int send(std::string msg)
	{
		for (auto &fd : _fds)
			sendTo(fd, msg);
		return 0;
	}

	int sendTo(int fd, std::string msg)
	{
		auto iter = std::find(_fds.begin(), _fds.end(), fd);
		if (iter == _fds.end())
			return 0;
		return ::send(fd, msg.c_str(), msg.size(), MSG_NOSIGNAL);
	}

	int start()
	{
		int err;

		_listenfd = socket(AF_INET, SOCK_STREAM, 0);
		if (_listenfd < 0)
			return -1;

		const int on = 1;
		setsockopt(_listenfd, SOL_SOCKET, SO_REUSEADDR,
				(const char*)&on, sizeof(on));
#ifdef SO_REUSEPORT
		setsockopt(_listenfd, SOL_SOCKET, SO_REUSEPORT,
				(const char*)&on, sizeof(on));
#endif

#ifdef SO_NOSIGPIPE
		setsockopt(_listenfd, SOL_SOCKET, SO_NOSIGPIPE,
				(const void *)&on, sizeof(on));
#endif

		struct sockaddr_in addr;
		memset(&addr, 0, sizeof(struct sockaddr_in));
		addr.sin_family = AF_INET;
		if (inet_aton(_addr.c_str(), &addr.sin_addr) == 0)
			addr.sin_addr.s_addr = htonl(INADDR_ANY);
		addr.sin_port = htons(_port);

		err = bind(_listenfd, (struct sockaddr*)&addr,
				sizeof(struct sockaddr_in));
		if (err < 0) {
			close(_listenfd);
			perror("bind");
			return -1;
		}

		err = listen(_listenfd, 64);
		if (err < 0) {
			close(_listenfd);
			perror("listen");
			return -1;
		}

		pipe(_pipe);

		FD_ZERO(&_read_set);
		FD_SET(_listenfd, &_read_set);
		FD_SET(_pipe[0], &_read_set);
		_maxfd = std::max(_listenfd, _pipe[0]);

		_eventThread = std::thread([this]{ eventLoop(); });
		_messageThread = std::thread([this]{ messageLoop(); });

		return 0;
	}

	void stop()
	{
		if (!_stop) {
			{
				std::unique_lock<std::mutex> lock(_queueMutex);
				_stop = true;
                /* we should handle all the request before exit */
				//while(_queue.size())
				//	_queue.pop();
				write(_pipe[1], "W", 1);
				_condition.notify_all();
			}
			_eventThread.join();
			_messageThread.join();
			close(_listenfd);
			close(_pipe[0]);
			close(_pipe[1]);
		}
	}

	SocketServer(std::string addr, int port)
	: _stop(false)
	, _addr(addr)
	, _port(port)
	{
	}

	~SocketServer()
	{
		stop();
	}

private:
	void eventLoop()
	{
		for (;;) {
			struct timeval timeout = {10, 0};

			fd_set copy_set = _read_set;

			int nready = select(_maxfd + 1, &copy_set, NULL, NULL, &timeout);
			if (_stop)
				return;

			if (nready == -1) {
				perror("select");
			} else if (nready == 0) {
				//std::cout << "Timeout" << std::endl;
			} else {
				if (FD_ISSET(_listenfd, &copy_set)) {
					//new client
					struct sockaddr_in addr;
					socklen_t len = sizeof(addr);

					int fd = accept(_listenfd, (struct sockaddr*)&addr, &len);
					if (fd > 1024) {
						printf("too much\n");
						continue;
					}
					if (fd == -1) {
						perror("accept");
					} else {
						FD_SET(fd, &_read_set);
						_maxfd = std::max(_maxfd, fd);
						_fds.push_back(fd);

						std::stringstream ss;
						ss << fd;
						enqueueMessage("Connect", ss.str());
					}
				}

				std::vector<int> to_remove;
				for (const auto &fd : _fds) {
					if (FD_ISSET(fd, &copy_set)) {
						//data from client
						char buf[4096];
						memset(buf, 0, sizeof(buf));
						int len = recv(fd, buf, sizeof(buf), 0);
						if (len <= 0) {
							//client disconnect
							to_remove.push_back(fd);
							continue;
						} else {
							std::stringstream ss;
							ss << fd;
							std::string msg(buf, len);
							enqueueMessage("Message", ss.str() + " " + msg);
						}

					}
				}

				for (auto &fd : to_remove) {
					FD_CLR(fd, &_read_set);
					_fds.erase(std::remove(_fds.begin(), _fds.end(), fd), _fds.end());
					_maxfd = std::max(_listenfd, _pipe[0]);
					for (auto iter = _fds.begin(); iter != _fds.end(); iter++) {
						_maxfd = std::max(_maxfd, *iter);
					}

					std::stringstream ss;
					ss << fd;
					enqueueMessage("Disconnect", ss.str());
				}
			}
		}
	}

	void messageLoop()
	{
		for (;;) {
			QueueMessage message;
			{
				std::unique_lock<std::mutex> lock(_queueMutex);
				_condition.wait(lock, [this]{ return _stop || !_queue.empty(); });
				if(_stop && _queue.empty())
					return;
				message = _queue.front();
				_queue.pop();
			}
			if (message.first == "Connect") {
				std::stringstream ss(message.second);
				int fd;
				ss >> fd;
				onConnect(fd);
			} else if (message.first == "Disconnect") {
				std::stringstream ss(message.second);
				int fd;
				ss >> fd;
				onDisconnect(fd);
				close(fd);
			} else if (message.first == "Message") {
				std::string msg = message.second;
				size_t pos = msg.find_first_of(" ");
				int fd = atoi(msg.substr(0, pos).c_str());
				onMessage(fd, msg.substr(pos+1));
			}
		}
	}

	void enqueueMessage(std::string event, std::string msg)
	{
		std::unique_lock<std::mutex> lock(_queueMutex);
		_queue.emplace(event, msg);
		_condition.notify_one();
	}

	std::thread _eventThread;
	std::thread _messageThread;
	typedef std::pair<std::string, std::string> QueueMessage;
	std::queue<QueueMessage> _queue;
	std::mutex _queueMutex;
	std::condition_variable _condition;
	int _listenfd;
	std::vector<int> _fds;
	fd_set _read_set;
	int _maxfd;
	bool _stop;
	std::string _addr;
	int _port;
	int _pipe[2];
};

#endif
