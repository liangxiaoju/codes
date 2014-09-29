#include "GameOverScene.h"
#include "GameScene.h"
#include "BGLayer.h"

using namespace CocosDenshion;

bool GameOverLayer::init() {
    bool bRet = false;

    do {
        CC_BREAK_IF(!Layer::init());

        Size s = Director::getInstance()->getVisibleSize();

		auto bg = BGLayer::create();
		addChild(bg);

        Sprite *s1_1 = Sprite::create("buttonBack.png");
        Sprite *s1_2 = Sprite::create("buttonBack.png");
		s1_2->setOpacity(160);
        MenuItemSprite *exit = MenuItemSprite::create(
                s1_1, s1_2, CC_CALLBACK_1(GameOverLayer::exitCallback, this));

        Sprite *s2_1 = Sprite::create("buttonRetry.png");
        Sprite *s2_2 = Sprite::create("buttonRetry.png");
		s2_2->setOpacity(160);
        MenuItemSprite *retry = MenuItemSprite::create(
                s2_1, s2_2, CC_CALLBACK_1(GameOverLayer::restartCallback, this));

        Sprite *s3_1 = Sprite::create("buttonLeaderboard.png");
        Sprite *s3_2 = Sprite::create("buttonLeaderboard.png");
		s3_2->setOpacity(160);
        MenuItemSprite *leaderboards = MenuItemSprite::create(
                s3_1, s3_2, CC_CALLBACK_1(GameOverLayer::leaderboardsCallback, this));

        mMenu = Menu::create(exit, retry, leaderboards, nullptr);
        mMenu->alignItemsHorizontallyWithPadding(50);
		mMenu->setPositionY(s1_1->getContentSize().height/2);
        addChild(mMenu);

		int highestScore = UserDefault::getInstance()->getIntegerForKey("highestScore", 0);
		int currentScore = UserDefault::getInstance()->getIntegerForKey("currentScore", 0);

		Sprite *s1 = Sprite::create("barTrackBorder.png");
		s1->setPosition(s.width/2, s.height*0.6);
		addChild(s1);
		char buf[128];
		snprintf(buf, sizeof(buf), "Highest: %d\nCurrent: %d", highestScore, currentScore);
        auto l1 = Label::createWithSystemFont(buf, "arial", 40);
		l1->enableOutline(Color4B::BLACK, 5);
		l1->setPosition(s.width/2, s.height*0.6);
		l1->setColor(Color3B::YELLOW);
		addChild(l1);

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
