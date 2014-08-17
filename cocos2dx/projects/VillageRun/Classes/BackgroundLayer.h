#ifndef __BACKGROUNDLAYER_H__
#define __BACKGROUNDLAYER_H__

#include "cocos2d.h"

USING_NS_CC;

class BackgroundLayer : public Layer {
public:
	virtual bool init();
	CREATE_FUNC(BackgroundLayer);
	void startMove();
	void stopMove();

private:
	void groundMove(float);
};

#endif
