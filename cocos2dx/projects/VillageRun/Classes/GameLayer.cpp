#include "GameLayer.h"
#include "HeroSprite.h"

bool GameLayer::init() {
	bool bRet = false;

	do {
		CC_BREAK_IF(!Layer::init());

		Size wsize = Director::getInstance()->getVisibleSize();

		HeroSprite *hero = HeroSprite::create();
		hero->setPosition(wsize.width/5, wsize.height*(2.0f/3));
		addChild(hero);

		auto map = TMXTiledMap::create("maps/basicLevels/basicLevel_1.tmx");
		addChild(map);

		bRet = true;
	} while(0);

	return bRet;
}
