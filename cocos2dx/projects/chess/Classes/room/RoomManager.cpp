#include "RoomManager.h"

RoomManager *RoomManager::_manager = NULL;

RoomManager *RoomManager::getInstance()
{
	if (_manager == NULL)
		_manager = new (std::nothrow) RoomManager();
	return _manager;
}

std::vector<RoomManager::RoomInfo> RoomManager::scanRoom(int max, int timeout)
{
	RoomScanner scanner;
	return scanner.scan(max, timeout);
}

RoomClient* RoomManager::joinRoom(std::string clientName, RoomInfo info)
{
	RoomClient *client = new (std::nothrow) RoomClient(
			clientName, info.host, 6000);
	int err = client->connect();
	if (err < 0) {
		delete client;
		client = NULL;
	}
	return client;
}

void RoomManager::leaveRoom(RoomClient *client)
{
	if (client) {
		client->disconnect();
		delete client;
	}
}

RoomServer *RoomManager::createRoom(std::string name, std::string desc)
{
	RoomServer *server = new (std::nothrow) RoomServer("", 6000);
	int err = server->start();
	if (err < 0) {
		delete server;
		server = NULL;
	}

	Broadcast::NotifyCallBack notify = [this, name, desc]
		(Broadcast *broadcast, const RoomPacket &packet) {
			RoomPacket reply;
			if (packet["FROM"] == "scanner") {
				reply["TYPE"] = "scan";
				reply["FROM"] = "manager";
				reply["CONTENT"] = "I am here.";
                reply["NAME"] = name;
                reply["DESC"] = desc;
				broadcast->send(reply);
			}
		};

	Broadcast *broadcast = new (std::nothrow) Broadcast(notify);
	broadcast->start();
	_roomPrivateMap[server] = broadcast;

	return server;
}

void RoomManager::destroyRoom(RoomServer *server)
{
	if (server) {
		Broadcast *broadcast = _roomPrivateMap[server];
		broadcast->stop();
		delete broadcast;

		server->stop();
		delete server;
	}
}

