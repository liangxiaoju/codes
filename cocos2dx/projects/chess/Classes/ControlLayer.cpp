#include "ControlLayer.h"

bool ControlLayer::init()
{
	if (!Layer::init())
		return false;

	auto vsize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();


	return true;
}
