#ifndef __GAMESCENE_H__
#define __GAMESCENE_H__

#include "cocos2d.h"
#include "GameLayer.h"

USING_NS_CC;

class GameScene : public Scene {
public:
	virtual bool init();
	CREATE_FUNC(GameScene);

private:
	GameLayer *mGameLayer;
};

#endif
