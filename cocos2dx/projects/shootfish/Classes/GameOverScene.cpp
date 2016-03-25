#include "GameOverScene.h"
#include "GameScene.h"
#include "JavaHelper.h"
#include "SimpleAudioEngine.h"

using namespace CocosDenshion;

bool GameOverLayer::init() {
    bool bRet = false;

    do {
        CC_BREAK_IF(!Layer::init());

        auto bg = Sprite::createWithSpriteFrameName("bg.png");
        bg->setAnchorPoint(Vec2(0, 0));
        bg->setPosition(Vec2(0, 0));
        addChild(bg);

        Size winSize = Director::getInstance()->getWinSize();

        char score[32];
        auto l1 = Label::createWithSystemFont("Highest", "Marker Felt", 50);
        auto l2 = Label::createWithSystemFont("Current", "Marker Felt", 50);
        snprintf(score, sizeof(score), "%d", ScoreLayer::getInstance()->getHighestScore());
        auto ls1 = Label::createWithBMFont("fonts/score.fnt", score);
        snprintf(score, sizeof(score), "%d", ScoreLayer::getInstance()->getCurrentScore());
        auto ls2 = Label::createWithBMFont("fonts/score.fnt", score);
        l1->setPosition(winSize.width/3, winSize.height*2/3);
        l2->setPosition(winSize.width*2/3, winSize.height*2/3);
        ls1->setPosition(winSize.width/3, winSize.height*2/3-80);
        ls2->setPosition(winSize.width*2/3, winSize.height*2/3-80);
        l1->setColor(Color3B::BLACK);
        l2->setColor(Color3B::BLACK);
        addChild(l1);
        addChild(l2);
        addChild(ls1);
        addChild(ls2);

        ScoreLayer::getInstance()->clear();

        Sprite *s1 = Sprite::create("ui/stop_64.png");
        Sprite *s1p = Sprite::create("ui/stop_64.png");
        s1p->setColor(Color3B::GRAY);
        MenuItemSprite *exit = MenuItemSprite::create(
                s1, s1p, CC_CALLBACK_1(GameOverLayer::exitCallback, this));

        Sprite *s2 = Sprite::create("ui/reload_64.png");
        Sprite *s2p = Sprite::create("ui/reload_64.png");
        s2p->setColor(Color3B::GRAY);
        MenuItemSprite *restart = MenuItemSprite::create(
                s2, s2p, CC_CALLBACK_1(GameOverLayer::restartCallback, this));

        Sprite *s3 = Sprite::create("ui/chart_bar_64.png");
        Sprite *s3p = Sprite::create("ui/chart_bar_64.png");
        s3p->setColor(Color3B::GRAY);
        MenuItemSprite *leaderboards = MenuItemSprite::create(
                s3, s3p, CC_CALLBACK_1(GameOverLayer::leaderboardsCallback, this));

        auto menu = Menu::create(exit, restart, leaderboards, nullptr);
        menu->alignItemsHorizontallyWithPadding(100);
        menu->setAnchorPoint(Vec2(0.5, 0.5));
        menu->setPosition(winSize.width/2, winSize.height/3);
        addChild(menu);

        SimpleAudioEngine::getInstance()->stopBackgroundMusic();
        SimpleAudioEngine::getInstance()->stopAllEffects(); 

        JavaHelper::getInstance()->showAds();
        JavaHelper::getInstance()->storeLeaderboards(ScoreLayer::getInstance()->getHighestScore());

        bRet = true;
    } while (0);

    return bRet;
}

void GameOverLayer::exitCallback(Ref *sender) {
    Director::getInstance()->end();
}

void GameOverLayer::restartCallback(Ref *sender) {
    JavaHelper::getInstance()->hideAds();
    Director::getInstance()->replaceScene(GameScene::create());
}

void GameOverLayer::leaderboardsCallback(Ref *sender) {
    JavaHelper::getInstance()->storeLeaderboards(ScoreLayer::getInstance()->getHighestScore());
    JavaHelper::getInstance()->showLeaderboards();
}
