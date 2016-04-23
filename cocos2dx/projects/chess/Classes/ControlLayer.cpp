#include "ControlLayer.h"

bool ControlLayer::init()
{
	if (!Layer::init())
		return false;

	auto visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	_menuSprite = Sprite::create();
	addChild(_menuSprite);
	_menuSprite->setAnchorPoint(Vec2::ANCHOR_BOTTOM_RIGHT);
	_menuSprite->setPosition(origin.x + visibleSize.width - 20, origin.y - 20);

	return true;
}
