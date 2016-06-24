#ifndef __NETPLAYER_H__
#define __NETPLAYER_H__

#include "cocos2d.h"
#include "Player.h"
#include "HeaderSprite.h"
#include "room/RoomManager.h"

USING_NS_CC;

class NetPlayer : public Player
{
public:
	virtual bool init();
	CREATE_FUNC(NetPlayer);

	virtual void ponder() {}
	virtual void go(float timeout) override;
	virtual void stop();
	virtual bool askForDraw();
	void setName(std::string first, std::string second);

	virtual ~NetPlayer();

private:
	HeaderSprite *_head;
	RoomClient *_client;
	std::queue<std::string> _queue;
	std::mutex _mutex;
	bool _active;
	int _sendcount;
};

#endif
