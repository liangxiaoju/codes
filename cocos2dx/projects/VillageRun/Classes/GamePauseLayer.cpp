#include "GamePauseLayer.h"
#include "GameScene.h"

bool GamePauseLayer::init() {
	bool bRet = false;

	do {
		CC_BREAK_IF(!Layer::init());

		auto listener = EventListenerTouchOneByOne::create();
		listener->onTouchBegan = [](Touch *,Event *) { return true; };
		listener->setSwallowTouches(true);
		_eventDispatcher->addEventListenerWithSceneGraphPriority(listener,this);

        Sprite *s1_1 = Sprite::createWithSpriteFrameName("buttonBack.png");
        Sprite *s1_2 = Sprite::createWithSpriteFrameName("buttonBack.png");
		s1_2->setOpacity(160);
        MenuItemSprite *exit = MenuItemSprite::create(
                s1_1, s1_2, CC_CALLBACK_1(GamePauseLayer::backCallback, this));

        Sprite *s2_1 = Sprite::createWithSpriteFrameName("buttonRetry.png");
        Sprite *s2_2 = Sprite::createWithSpriteFrameName("buttonRetry.png");
		s2_2->setOpacity(160);
        MenuItemSprite *retry = MenuItemSprite::create(
                s2_1, s2_2, CC_CALLBACK_1(GamePauseLayer::retryCallback, this));

        auto menu = Menu::create(exit, retry, nullptr);
        menu->alignItemsHorizontallyWithPadding(100);
		menu->setPositionY(s1_1->getContentSize().height/2);
        addChild(menu);

		bRet = true;
	} while(0);

	return bRet;
}

void GamePauseLayer::backCallback(Ref *sender) {
	Director::getInstance()->resume();
	removeFromParent();
}

void GamePauseLayer::retryCallback(Ref *sender) {
	Director::getInstance()->resume();
    Director::getInstance()->replaceScene(GameScene::create());
}

