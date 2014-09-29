#ifndef __GAMEPAUSELAYER_H__
#define __GAMEPAUSELAYER_H__

#include "cocos2d.h"

USING_NS_CC;

class GamePauseLayer : public Layer {
public:
	virtual bool init();
	CREATE_FUNC(GamePauseLayer);
private:
	void backCallback(Ref *sender);
	void retryCallback(Ref *sender);
};

#endif
