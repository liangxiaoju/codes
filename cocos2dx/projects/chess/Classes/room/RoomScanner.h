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

class SendBroadcast {
private:
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

	int send_broadcast()
	{
		RoomPacket packet;
		packet["TYPE"] = "scan";
		packet["FROM"] = "scanner";
		packet["CONTENT"] = "Where are you ?";
		std::string msg = packet.toString();

		do {
			struct sockaddr_in addrto[5];
			int num = getBroadcastAddr(addrto, 5);
			if (num <= 0) {
				printf("Failed to get broadcast address.\n");
				return -1;
			}

			int nlen = sizeof(struct sockaddr_in);
			int i, ret;

			for (i = 0; i < num; i++) {
				addrto[i].sin_port = htons(SCAN_PORT);

				ret = sendto(_sockfd,
						msg.c_str(), msg.size(),
						0, (struct sockaddr*)&addrto[i], nlen);  
				if (_stop)
					return 0;

				if (ret < 0) {
					perror("sendto");
				}
				sleep(1);
			}
		} while(1);

		return 0;  
	}

public:
	SendBroadcast() : _stop(false) {}

	int start() {
		if ((_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)   
		{     
			perror("socket");
			return -1;
		}

		const int opt = 1;  
		int nb = 0;  
		nb = setsockopt(_sockfd, SOL_SOCKET, SO_BROADCAST,
				(char *)&opt, sizeof(opt));  
		if(nb == -1)  
		{  
			perror("setsockopt");
			return -1;  
		}

		_thread = std::thread([this]{send_broadcast();});

		return 0;
	}

	void stop() {
		_stop = true;
		shutdown(_sockfd, SHUT_RDWR);
		_thread.join();
		close(_sockfd);
	}

private:
	int _stop;
	int _sockfd;
	std::thread _thread;
};

class RecvBroadcast {
private:
	int isLocalAddr(struct sockaddr_in *addr)
	{
		struct ifaddrs *ifaddr, *ifa;
		int status = 0;

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

			int family = ifa->ifa_addr->sa_family;
			if (family != AF_INET)
				continue;

			if (addr->sin_addr.s_addr ==
					((struct sockaddr_in *)(ifa->ifa_addr))->sin_addr.s_addr) {
				status = 1;
				break;
			}
		}

		freeifaddrs(ifaddr);

		return status;
	}

	int recv_broadcast()
	{
		do {
			int err;
			int len = sizeof(struct sockaddr_in);  
			char msg[4096];

			struct sockaddr_in from;  
			bzero(&from, sizeof(struct sockaddr_in));  

			err = recvfrom(_sockfd, msg, sizeof(msg), 0,
					(struct sockaddr*)&from, (socklen_t*)&len);  
			if (_stop)
				break;

			if (err < 0) {
				perror("recvfrom");
				break;
			} else if (err == 0) {
				continue;
			}

			//if (isLocalAddr(&from))
			//	continue;

			//printf("add: %s\n", inet_ntoa(from.sin_addr));
			RoomPacket packet(msg);
			packet["ADDR"] = inet_ntoa(from.sin_addr);
			_notify(this, packet);

		} while (1);

		return 0;
	}

public:
	typedef std::function<void(RecvBroadcast *, const RoomPacket &packet)> NotifyCallBack;

	RecvBroadcast(NotifyCallBack &notify)
	: _stop(false)
	{
		_notify = notify;
	}

	int start() {
		if ((_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)   
		{     
			perror("socket");
			return -1;  
		}

		const int on = 1;
		setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR,
				(const char*)&on, sizeof(on));

		struct sockaddr_in addr;  
		bzero(&addr, sizeof(struct sockaddr_in));  
		addr.sin_family = AF_INET;  
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
		addr.sin_port = htons(SCAN_PORT);

		if(bind(_sockfd,(struct sockaddr *)&(addr),
					sizeof(struct sockaddr_in)) == -1) {
			perror("bind");
			return -1;
		}

		_thread = std::thread([this]{recv_broadcast();});

		return 0;
	}

	void stop() {
		_stop = true;
		shutdown(_sockfd, SHUT_RDWR);
		_thread.join();
		close(_sockfd);
	}

	int reply(RoomPacket &packet)
	{
		struct sockaddr_in addrto;
		std::string msg = packet.toString();

		memset(&addrto, 0, sizeof(addrto));
		addrto.sin_family = AF_INET;
		inet_aton(packet["TO"].c_str(), &addrto.sin_addr);
		addrto.sin_port = htons(SCAN_PORT);

		return sendto(_sockfd, msg.c_str(), msg.size(), 0,
				(struct sockaddr*)&addrto, sizeof(struct sockaddr_in));
	}

private:
	int _stop;
	int _sockfd;
	std::thread _thread;
	NotifyCallBack _notify;
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

		RecvBroadcast::NotifyCallBack notify = [this, &infos, max]
			(RecvBroadcast *receiver, const RoomPacket &packet) {

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

		RecvBroadcast receiver(notify);
		receiver.start();
		SendBroadcast sender;
		sender.start();

		std::unique_lock<std::mutex> lock(_mutex);
#if 0
		_condition.wait(lock, [&infos, max]{ return infos.size() >= max; });
#else
		if (infos.size() < max)
			_condition.wait_for(lock, std::chrono::seconds(timeout));
#endif

		sender.stop();
		receiver.stop();

		return infos;
	}

private:
	std::mutex _mutex;
	std::condition_variable _condition;
};

#endif
