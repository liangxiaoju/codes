#ifndef __NETPLAYER_H__
#define __NETPLAYER_H__

#include "cocos2d.h"
#include "Player.h"
#include "Utils.h"
#include "room/RoomManager.h"
#include <memory>  // for std::shared_ptr
#include <atomic>

USING_NS_CC;

class NetPlayer : public Player
{
public:
	virtual bool init(RoomClient *client);
	static NetPlayer* create(RoomClient *client)
	{
		NetPlayer *pRet = new(std::nothrow) NetPlayer();
		if (pRet && pRet->init(client))
		{
			pRet->autorelease();
			return pRet;
		}
		else
		{
			delete pRet;
			pRet = nullptr;
			return nullptr;
		}
	}

	virtual void start(std::string fen) override;
	virtual void stop() override;
	virtual void onRequest(std::string req, std::string args="",
                           std::function<void(bool)>callback=nullptr) override;
    virtual void onReply(std::string reply, std::string args="") override;

	virtual ~NetPlayer();

private:
	RoomClient *_client;
	std::queue<std::string> _queue;
	std::mutex _mutex;
	bool _active;
	std::string _fen;
	int _sendcount;
    std::shared_ptr<std::atomic<bool>> _isDestroyed;
};

#endif
