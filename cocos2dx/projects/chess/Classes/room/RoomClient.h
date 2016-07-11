#ifndef __ROOMCLIENT_H__
#define __ROOMCLIENT_H__

#include <queue>
#include <mutex>
#include <condition_variable>
#include <unordered_map>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <thread>

#include "SocketClient.h"
#include "RoomPacket.h"

class RoomClient : public SocketClient
{
public:
	typedef std::function<void(RoomClient*, const RoomPacket&)> RoomEvent;

	RoomClient(std::string name, std::string addr, int port)
	: SocketClient(addr, port)
	, _name(name)
	{
	}

    void onError(std::string err)
    {
        RoomPacket packet;
        packet["TYPE"] = "error";
        packet["CONTENT"] = err;
		packet["FROM"] = _name;
		packet["TO"] = _name;
        fireEvent(packet);
    }

	void onMessage(std::string msg)
	{
		auto packets = RoomPacket::parser(msg);

		//std::cout << "onMessage: " << msg << std::endl;

		for (auto &packet : packets) {

			std::string from = packet["FROM"];

			if (from == "admin") {
				std::string type = packet["TYPE"];
				if (type == "ask") {
					RoomPacket reply;
					reply["TYPE"] = "ack";
					reply["TO"] = "admin";
					emitPacket(reply);
				} else if (type == "connect") {
					_members.push_back(packet["CONTENT"]);
				} else if (type == "disconnect") {
					std::string who = packet["CONTENT"];
					auto iter = std::find(_members.begin(), _members.end(), who);
					if (iter != _members.end())
						_members.erase(iter);
				}
			}

			fireEvent(packet);
		}
	}

	int emitPacket(RoomPacket &packet)
	{
		packet["FROM"] = _name;
		return send(packet.toString());
	}

	void broadcastMessage(std::string msg)
	{
		RoomPacket packet;
		packet["TYPE"] = "message";
		packet["TO"] = "all";
		packet["CONTENT"] = msg;
		emitPacket(packet);
	}

	void emit(std::string event, std::string arg)
	{
		RoomPacket packet;
		packet["TYPE"] = event;
		packet["TO"] = "all";
		packet["CONTENT"] = arg;
		emitPacket(packet);
	}

	int on(const std::string& eventName, RoomEvent e)
	{
		_eventRegistry[eventName] = e;
		return 0;
	}

	std::vector<std::string> getMemberList()
	{
		return _members;
	}

private:
	void fireEvent(RoomPacket &packet)
	{
		std::string event = packet["TYPE"];
		if (_eventRegistry[event]) {
			auto e = _eventRegistry[event];
			e(this, packet);
		}
	}

	std::string _name;
	std::vector<std::string> _members;
	std::unordered_map<std::string, RoomEvent> _eventRegistry;
};

#endif
