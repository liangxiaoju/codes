#include "GameOverScene.h"
#include "GameScene.h"

using namespace CocosDenshion;

bool GameOverLayer::init() {
    bool bRet = false;

    do {
        CC_BREAK_IF(!Layer::init());
/*
        auto bg = Sprite::createWithSpriteFrameName("background.png");
        bg->setAnchorPoint(Vec2(0, 0));
        bg->setPosition(Vec2(0, 0));
        addChild(bg);
*/
        Size winSize = Director::getInstance()->getWinSize();
        //mLabel = Label::createWithBMFont("fonts/score.fnt", "0");

        mMenu = Menu::create();
        MenuItemFont::setFontSize(30);
        mMenu->addChild(MenuItemFont::create(
                    "Exit", CC_CALLBACK_1(GameOverLayer::exitCallback, this)));
        mMenu->addChild(MenuItemFont::create(
                    "Restart", CC_CALLBACK_1(GameOverLayer::restartCallback, this)));
        mMenu->addChild(MenuItemFont::create(
                    "Leaderboards", CC_CALLBACK_1(GameOverLayer::leaderboardsCallback, this)));
        mMenu->alignItemsHorizontallyWithPadding(30);
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
