#include "GameLayer.h"
#include "HeroSprite.h"
#include "GameStatusLayer.h"

bool GameLayer::init() {
	bool bRet = false;

	do {
		CC_BREAK_IF(!Layer::init());

		Size wsize = Director::getInstance()->getVisibleSize();

		mBackgroundLayer = BackgroundLayer::create();
		addChild(mBackgroundLayer, 0, "background");

		HeroSprite *hero = HeroSprite::create();
		hero->setPosition(wsize.width/5, wsize.height*(2.0f/3));
		addChild(hero);

		GameStatusLayer *status = GameStatusLayer::create();
		addChild(status, 0, "gamestatus");

		bRet = true;
	} while(0);

	return bRet;
}
