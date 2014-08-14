#include "GameScene.h"

USING_NS_CC;

GameScene::GameScene() {
}

GameScene::~GameScene() {
}

bool GameScene::init() {
    bool bRet = false;

    do {
        CC_BREAK_IF(!Scene::init());

        mBackgroundLayer = BackgroundLayer::create();
        addChild(mBackgroundLayer);

        mGameLayer = GameLayer::getInstance();
        addChild(mGameLayer);

        mScoreLayer = ScoreLayer::getInstance();
        addChild(mScoreLayer);

        mMenuLayer = MenuLayer::create();
        addChild(mMenuLayer);

        bRet = true;
    } while (0);

    return bRet;
}

void GameScene::onExit() {
    removeAllChildren();
    Scene::onExit();
}
