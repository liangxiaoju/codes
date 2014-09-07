#include "GameOverScene.h"
#include "GameScene.h"
#include "BackgroundLayer.h"

using namespace CocosDenshion;

bool GameOverLayer::init() {
    bool bRet = false;

    do {
        CC_BREAK_IF(!Layer::init());

        Size winSize = Director::getInstance()->getWinSize();

		auto bg = BackgroundLayer::create();
		bg->stopMove();
		addChild(bg);

        Sprite *s1_1 = Sprite::createWithSpriteFrameName("buttonBack.png");
        Sprite *s1_2 = Sprite::createWithSpriteFrameName("buttonBack.png");
		s1_2->setOpacity(160);
        MenuItemSprite *exit = MenuItemSprite::create(
                s1_1, s1_2, CC_CALLBACK_1(GameOverLayer::exitCallback, this));

        Sprite *s2_1 = Sprite::createWithSpriteFrameName("buttonRetry.png");
        Sprite *s2_2 = Sprite::createWithSpriteFrameName("buttonRetry.png");
		s2_2->setOpacity(160);
        MenuItemSprite *retry = MenuItemSprite::create(
                s2_1, s2_2, CC_CALLBACK_1(GameOverLayer::restartCallback, this));

        Sprite *s3_1 = Sprite::createWithSpriteFrameName("buttonLeaderboard.png");
        Sprite *s3_2 = Sprite::createWithSpriteFrameName("buttonLeaderboard.png");
		s3_2->setOpacity(160);
        MenuItemSprite *leaderboards = MenuItemSprite::create(
                s3_1, s3_2, CC_CALLBACK_1(GameOverLayer::leaderboardsCallback, this));

        mMenu = Menu::create(exit, retry, leaderboards, nullptr);
        mMenu->alignItemsHorizontallyWithPadding(50);
		mMenu->setPositionY(s1_1->getContentSize().height/2);
        addChild(mMenu);

        bRet = true;
    } while (0);

    return bRet;
}

void GameOverLayer::exitCallback(Ref *sender) {
    Director::getInstance()->end();
}

void GameOverLayer::restartCallback(Ref *sender) {
    Director::getInstance()->replaceScene(GameScene::create());
}

void GameOverLayer::leaderboardsCallback(Ref *sender) {
}
