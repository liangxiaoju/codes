#include "BGLayer.h"

bool BGLayer::init()
{
	if (!Layer::init())
		return false;

	auto sprite = Sprite::create("common_bg_new2.png");
	sprite->setAnchorPoint(Vec2(0, 0));
	sprite->setPosition(Vec2(0, 0));
	addChild(sprite);

	return true;
}
