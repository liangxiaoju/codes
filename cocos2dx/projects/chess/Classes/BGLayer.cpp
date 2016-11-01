#include "BGLayer.h"

bool BGLayer::init()
{
	if (!Layer::init())
		return false;

	auto sprite = Sprite::create("common/background.png");
	sprite->setAnchorPoint(Vec2(0, 0));
	sprite->setPosition(Vec2(0, 0));
	addChild(sprite);

	return true;
}
