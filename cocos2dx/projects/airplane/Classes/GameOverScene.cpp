#include "GameOverScene.h"
#include "GameScene.h"
#include "AdController.h"

using namespace CocosDenshion;

bool GameOverLayer::init() {
    bool bRet = false;

    do {
        CC_BREAK_IF(!Layer::init());

        auto bg = Sprite::createWithSpriteFrameName("background.png");
        bg->setAnchorPoint(Vec2(0, 0));
        bg->setPosition(Vec2(0, 0));
        addChild(bg);

        Size winSize = Director::getInstance()->getWinSize();
        //mLabel = Label::createWithBMFont("fonts/score.fnt", "0");
        mLabel = Label::createWithSystemFont("","Marker Felt",30);
        mLabel->setAnchorPoint(Vec2(0.5, 0.5));
        mLabel->setPosition(winSize.width/2, winSize.height*2/3);
        char score[32];
        snprintf(score, sizeof(score), "Best %d\nScore %d\n",
                ScoreLayer::getInstance()->getHighestScore(),
                ScoreLayer::getInstance()->getCurrentScore());
        mLabel->setAlignment(TextHAlignment::CENTER, TextVAlignment::CENTER);
        //mLabel->setTextColor(Color4B::BLACK);
        mLabel->setString(score);
        addChild(mLabel);
        ScoreLayer::getInstance()->clear();

        mMenu = Menu::create();
        MenuItemFont::setFontSize(30);
        mMenu->addChild(MenuItemFont::create(
                    "Exit", CC_CALLBACK_1(GameOverLayer::exitCallback, this)));
        mMenu->addChild(MenuItemFont::create(
                    "Restart", CC_CALLBACK_1(GameOverLayer::restartCallback, this)));
        mMenu->alignItemsHorizontallyWithPadding(30);
        addChild(mMenu);

        SimpleAudioEngine::getInstance()->stopBackgroundMusic();
        SimpleAudioEngine::getInstance()->stopAllEffects(); 

        AdController::getInstance()->setVisible(true);

        bRet = true;
    } while (0);

    return bRet;
}

void GameOverLayer::exitCallback(Ref *sender) {
    Director::getInstance()->end();
}

void GameOverLayer::restartCallback(Ref *sender) {
    AdController::getInstance()->setVisible(false);
    Director::getInstance()->replaceScene(GameScene::create());
}
