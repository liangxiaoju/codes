#include "GameScene.h"

USING_NS_CC;

bool GameScene::init() {
    bool bRet = false;

    do {
        CC_BREAK_IF(!Scene::init());

        mBackgroundLayer = BackgroundLayer::create();
        addChild(mBackgroundLayer);

        mGameLayer = GameLayer::getInstance();
        mGameLayer->restart();
        addChild(mGameLayer);

        mScoreLayer = ScoreLayer::getInstance();
        addChild(mScoreLayer);

        bRet = true;
    } while (0);

    return bRet;
}

void GameScene::onEnter() {
    Scene::onEnter();
}

void GameScene::onExit() {
    //removeAllChildren();
    Scene::onExit();
}

