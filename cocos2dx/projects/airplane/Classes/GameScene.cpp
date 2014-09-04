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
        mGameLayer->restart();
        addChild(mGameLayer);

        mScoreLayer = ScoreLayer::getInstance();
        addChild(mScoreLayer);

        auto keyListener = EventListenerKeyboard::create();
        keyListener->onKeyPressed = CC_CALLBACK_2(GameScene::onKeyPressed, this);
        keyListener->onKeyReleased = CC_CALLBACK_2(GameScene::onKeyReleased, this);
        Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(keyListener, this);

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

void GameScene::onKeyPressed(EventKeyboard::KeyCode keyCode, Event *unused) {
}

void GameScene::onKeyReleased(EventKeyboard::KeyCode keyCode, Event *unused) {
    switch (keyCode) {
    case EventKeyboard::KeyCode::KEY_BACK: {
        DelayTime *delay = DelayTime::create(0.01f);
        CallFunc *callback = CallFunc::create([&]() {
                Size s = Director::getInstance()->getVisibleSize();
                auto texture = RenderTexture::create(s.width, s.height);
                texture->begin();
                this->visit();
                texture->end();
                Director::getInstance()->pushScene(MenuScene::create(texture));
        });
        Sequence *seq = Sequence::create(delay, callback, nullptr);
        runAction(seq);
        break;
        }
    default:
        break;
    }
}
