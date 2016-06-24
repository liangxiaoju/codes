#ifndef __ROOMMANAGER_H__
#define __ROOMMANAGER_H__

#include <iostream>
#include <vector>
#include <functional>

#include "RoomClient.h"
#include "RoomServer.h"
#include "RoomScanner.h"

#define ROOM_PORT 6000

class RoomManager
{
public:
	typedef RoomScanner::RoomInfo RoomInfo;

	static RoomManager *getInstance();
	std::vector<RoomInfo> scanRoom(int max = 1, int timeout = 5);
	RoomClient* joinRoom(std::string clientName, RoomInfo info);
	void leaveRoom(RoomClient *client);
	RoomServer *createRoom();
	void destroyRoom(RoomServer *server);

private:
	static RoomManager *_manager;
	std::unordered_map<RoomServer*, RecvBroadcast*> _roomPrivateMap;
};

#endif
