#ifndef __LANFIGHTSCENE_H__
#define __LANFIGHTSCENE_H__

#include "cocos2d.h"
#include "Player.h"
#include "UIPlayer.h"
#include "NetPlayer.h"
#include "Board.h"
#include "GameLayer.h"
#include "room/RoomManager.h"

USING_NS_CC;

class LANFightScene : public Scene
{
public:
	bool init();
	CREATE_FUNC(LANFightScene);

private:
	void scan();

	UIPlayer *_playerUI;
	NetPlayer *_playerNet;
	Player *_playerWhite;
	Player *_playerBlack;
	GameLayer *_gameLayer;
	Board *_board;
	RoomClient *_client;
	RoomServer *_server;
	std::thread _thread;
	bool _rotation;
};

#endif
