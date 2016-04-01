#ifndef __BGLAYER_H__
#define __BGLAYER_H__

#include "cocos2d.h"

USING_NS_CC;

class BGLayer : public Layer
{
public:
	virtual bool init() override;
	CREATE_FUNC(BGLayer);
};

#endif
