#ifndef __AIPLAYER_H__
#define __AIPLAYER_H__

#include "cocos2d.h"
#include "Player.h"

USING_NS_CC;

class AIPlayer : public Player
{
public:
	virtual void ponder() override;
	virtual void go(float timeout) override;

private:
};

#endif
