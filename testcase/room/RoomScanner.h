#ifndef __ROOMSCANNER_H__
#define __ROOMSCANNER_H__

#include <thread>
#include <condition_variable>

#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <net/if.h>

#include "ifaddrs.h"
#include "RoomPacket.h"

#define SCAN_PORT 6001

class Broadcast
{
public:
	typedef std::function<void(Broadcast*,const RoomPacket&)> NotifyCallBack;

	Broadcast()
	: _stop(false)
	, _mode(0)
	{}

	Broadcast(NotifyCallBack &notify)
	: _stop(false)
	, _notify(notify)
	, _mode(MODE_R)
	{}

	Broadcast(RoomPacket &packet)
	: _stop(false)
	, _packet(packet)
	, _mode(MODE_W)
	{}

	Broadcast(NotifyCallBack &notify, RoomPacket &packet)
	: _stop(false)
	, _notify(notify)
	, _packet(packet)
	, _mode(MODE_R|MODE_W)
	{}

	void start()
	{
		setUp();

		if (_mode & MODE_R) {
			_readThread = std::thread([this]{readThread();});
		}
		if (_mode & MODE_W) {
			_sendThread = std::thread([this]{sendThread();});
		}
	}

	void stop()
	{
		tearDown();
	}

	int send(RoomPacket &packet)
	{
		std::string msg = packet.toString();

		struct sockaddr_in addrto[5];
		int num = getBroadcastAddr(addrto, 5);
		if (num <= 0) {
			printf("Failed to get broadcast address.\n");
			return -1;
		}

		int nlen = sizeof(struct sockaddr_in);
		int i, ret;

		std::unique_lock<std::mutex> lock(_mutex_w);

		for (i = 0; i < num; i++) {
			addrto[i].sin_port = htons(SCAN_PORT);

			ret = sendto(_sockfd,
					msg.c_str(), msg.size(),
					0, (struct sockaddr*)&addrto[i], nlen);  
			if (ret < 0) {
				perror("sendto");
				return -1;
			}
		}

		return 0;
	}

	int read(RoomPacket &packet)
	{
		int err;
		int len = sizeof(struct sockaddr_in);  
		char msg[4096];

		struct sockaddr_in from;  
		memset(&from, 0, sizeof(from));

		fd_set fdset;
		FD_ZERO(&fdset);
		FD_SET(_sockfd, &fdset);
		FD_SET(_pipe[0], &fdset);

		select(std::max(_sockfd, _pipe[0])+1, &fdset, NULL, NULL, NULL);

		if (FD_ISSET(_sockfd, &fdset)) {
			std::unique_lock<std::mutex> lock(_mutex_r);

			err = recvfrom(_sockfd, msg, sizeof(msg), 0,
					(struct sockaddr*)&from, (socklen_t*)&len);  
			if (err < 0) {
				perror("recvfrom");
				return -1;
			}
		}

		printf("add: %s\n", inet_ntoa(from.sin_addr));
		packet = RoomPacket(msg);
		packet["ADDR"] = inet_ntoa(from.sin_addr);

		return 0;
	}

private:
	enum {
		MODE_R = 0x1,
		MODE_W = 0x2,
	};

	int setUp()
	{
		int fd, err;

		fd = socket(AF_INET, SOCK_DGRAM, 0);
		if (fd < 0) {
			perror("socket");
			return -1;
		}

		const int on = 1;
		setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
				(const char*)&on, sizeof(on));
#ifdef SO_REUSEPORT
		setsockopt(fd, SOL_SOCKET, SO_REUSEPORT,
				(const char*)&on, sizeof(on));
#endif

		struct sockaddr_in addr;
		memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
		addr.sin_port = htons(SCAN_PORT);

		err = bind(fd, (struct sockaddr*)&addr, sizeof(addr));
		if (err < 0) {
			perror("bind");
			close(fd);
			return -1;
		}

		const int opt = 1;  
		err = setsockopt(fd, SOL_SOCKET, SO_BROADCAST,
				(char *)&opt, sizeof(opt));  
		if (err < 0) {
			perror("setsockopt");
			close(fd);
			return -1;
		}

		pipe(_pipe);

		_sockfd = fd;

		return 0;
	}

	void tearDown()
	{
		_stop = true;
		write(_pipe[1], "W", 1);
		if (_mode & MODE_W)
			_sendThread.join();
		if (_mode & MODE_R)
			_readThread.join();
		close(_pipe[0]);
		close(_pipe[1]);
		close(_sockfd);
	}

	int getBroadcastAddr(struct sockaddr_in *addrs, int maxsize)
	{
		struct ifaddrs *ifaddr = NULL, *ifa;
		int family, err, size = 0;

		if (getifaddrs(&ifaddr) == -1) {
			perror("getifaddrs");
			return -1;
		}

		for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
			if (ifa->ifa_addr == NULL)
				continue;

			if (!(ifa->ifa_flags & IFF_UP))
				continue;

			if (ifa->ifa_flags & IFF_LOOPBACK)
				continue;

			if (!(ifa->ifa_flags & IFF_BROADCAST))
				continue;

			family = ifa->ifa_addr->sa_family;
			if (family != AF_INET)
				continue;

			if (size >= maxsize)
				break;

			memcpy(&addrs[size], ifa->ifa_broadaddr, sizeof(struct sockaddr_in));
			size++;
		}

		freeifaddrs(ifaddr);

		return size;
	}

	void sendThread()
	{
		std::cout << "sendThread start" << std::endl;
		do {
			send(_packet);
			if (_stop)
				break;
			sleep(1);
		} while (1);
		std::cout << "sendThread exit" << std::endl;
	}

	void readThread()
	{
		RoomPacket packet;

		std::cout << "readThread start" << std::endl;
		do {
			read(packet);
			if (_stop)
				break;
			_notify(this, packet);
		} while (1);
		std::cout << "readThread exit" << std::endl;
	}

	bool _stop;
	int _sockfd;
	std::thread _sendThread;
	std::thread _readThread;
	NotifyCallBack _notify;
	RoomPacket _packet;
	int _mode;
	std::mutex _mutex_w;
	std::mutex _mutex_r;
	int _pipe[2];
};

class RoomScanner
{
public:
	struct RoomInfo
	{
		std::string name;
		std::string desc;
		std::string host;
	};

	std::vector<RoomInfo> scan(int max = 1, int timeout = 5)
	{
		std::vector<RoomInfo> infos;

		Broadcast::NotifyCallBack notify = [this, &infos, max]
			(Broadcast *broadcast, const RoomPacket &packet) {

				if (packet["FROM"] == "scanner") {
					return;
				}

				for (auto &v : infos) {
					if (v.name == packet["NAME"] &&
							v.name == packet["ADDR"])
						return;
				}

				RoomInfo info;
				info.name = packet["NAME"];
				info.desc = packet["DESC"];
				info.host = packet["ADDR"];
				infos.push_back(info);

				if (infos.size() >= max) {
					_condition.notify_one();
				}
		};

		RoomPacket packet;
		packet["TYPE"] = "scan";
		packet["FROM"] = "scanner";
		packet["CONTENT"] = "Where are you ?";

		Broadcast broadcast(notify, packet);
		broadcast.start();

		std::unique_lock<std::mutex> lock(_mutex);
		if (infos.size() < max)
			_condition.wait_for(lock, std::chrono::seconds(timeout));

		broadcast.stop();

		return infos;
	}

private:
	std::mutex _mutex;
	std::condition_variable _condition;
};

#endif
