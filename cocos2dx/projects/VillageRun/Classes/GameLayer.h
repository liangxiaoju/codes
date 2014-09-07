#ifndef __GAMELAYER_H__
#define __GAMELAYER_H__

#include "cocos2d.h"
#include "BackgroundLayer.h"

USING_NS_CC;

class GameLayer : public Layer {
public:
	virtual bool init();
	CREATE_FUNC(GameLayer);

private:
	BackgroundLayer *mBackgroundLayer;
};

#endif
