#ifndef __CONTROLLAYER_H__
#define __CONTROLLAYER_H__

#include "cocos2d.h"

USING_NS_CC;

class ControlLayer : public Layer
{
public:
	virtual bool init();
	CREATE_FUNC(ControlLayer);

private:
	Sprite *_menuSprite;
};

#endif
