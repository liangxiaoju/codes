#ifndef __ROOMSERVER_H__
#define __ROOMSERVER_H__

#include <iostream>
#include <unordered_map>

#include "SocketServer.h"
#include "RoomPacket.h"

class RoomServer : public SocketServer
{
public:
	RoomServer(std::string addr, int port)
	: SocketServer(addr, port)
	{
	}

	int emitPacketTo(int fd, RoomPacket &packet)
	{
		return sendTo(fd, packet.toString());
	}

	int emitPacket(RoomPacket &packet)
	{
		for (auto &kv : _clientMap) {
			packet["TO"] = kv.second;
			if (emitPacketTo(kv.first, packet) < 0) {
				perror("send");
			}
		}
		return 0;
	}

	void onConnect(int fd)
	{
		RoomPacket packet;
		packet["TYPE"] = "ask";
		packet["FROM"] = "admin";
		packet["TO"] = "unknown";
		emitPacketTo(fd, packet);
	}

	void onDisconnect(int fd)
	{
		auto iter = _clientMap.find(fd);
		if (iter != _clientMap.end()) {
			std::cout << "Del member: " << _clientMap[fd] << std::endl;
			_clientMap.erase(iter);
			RoomPacket packet;
			packet["TYPE"] = "disconnect";
			packet["FROM"] = "admin";
			emitPacket(packet);
		}
	}

	void onMessage(int fd, std::string msg)
	{
		auto packets = RoomPacket::parser(msg);

		//std::cout << "onMessage: " << msg << std::endl;

		for (auto &packet : packets) {

			std::string to = packet["TO"];
			std::string from = packet["FROM"];


			if (to == "admin") {
				if (packet["TYPE"] == "ack") {
                    /* tell others about the newer */
					RoomPacket p;
					p["TYPE"] = "connect";
					p["FROM"] = "admin";
					p["CONTENT"] = from;
					emitPacket(p);

                    /* send member list to the newer */
					for (auto &kv : _clientMap) {
						RoomPacket p;
						p["TYPE"] = "connect";
						p["FROM"] = "admin";
						p["CONTENT"] = kv.second;
						emitPacketTo(fd, p);
					}

					_clientMap[fd] = from;
					std::cout << "Add member: " << from << std::endl;
				}

			} else {
				if (from != _clientMap[fd])
					return;

				if (to == "all") {
					for (auto &kv : _clientMap)
						if (kv.first != fd)
							emitPacketTo(kv.first, packet);
				} else {
					for (auto &kv : _clientMap)
						if (kv.second == to)
							emitPacketTo(kv.first, packet);
				}
			}
		}
	}

private:
	std::unordered_map<int, std::string> _clientMap;
};

#endif
