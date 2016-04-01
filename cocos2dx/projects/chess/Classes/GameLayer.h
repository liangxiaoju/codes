#ifndef __GAMELAYER_H__
#define __GAMELAYER_H__

#include "cocos2d.h"
#include "Player.h"

USING_NS_CC;

class GameLayer : public cocos2d::Layer
{
public:
	virtual bool init() override;
	CREATE_FUNC(GameLayer);

private:
	void onPlayerWhiteMoved(std::string mv);
	void onPlayerBlackMoved(std::string mv);
	Player *_playerWhite;
	Player *_playerBlack;
};

#endif
