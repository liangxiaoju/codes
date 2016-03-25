#include "BackgroundLayer.h"
#include "SimpleAudioEngine.h"

using namespace CocosDenshion;

bool BackgroundLayer::init() {
    bool bRet = false;

    do {
        CC_BREAK_IF(!Layer::init());

        mBackground1 = Sprite::createWithSpriteFrameName("bg.png");
        mBackground2 = Sprite::createWithSpriteFrameName("bg.png");
        mBackground1->setAnchorPoint(Vec2(0, 0));
        mBackground1->setPosition(Vec2(0, 0));
        mBackground2->setAnchorPoint(Vec2(0, 0));
        mBackground2->setPosition(Vec2(0, mBackground1->getContentSize().height-1));

        addChild(mBackground1);
        addChild(mBackground2);

        startMove();

        bRet = true;
    } while (0);

    return bRet;
}

void BackgroundLayer::startMove() {
    schedule(schedule_selector(BackgroundLayer::move), 0.01f);
    SimpleAudioEngine::getInstance()->playBackgroundMusic("sound/game_music.mp3", true);
}

void BackgroundLayer::stopMove() {
    unschedule(schedule_selector(BackgroundLayer::move));
}

void BackgroundLayer::move(float dt) {
    Vec2 pos1 = mBackground1->getPosition();

    mBackground1->setPosition(Vec2(pos1.x, pos1.y-2));
    mBackground2->setPosition(Vec2(pos1.x,
                mBackground1->getPositionY()+mBackground1->getContentSize().height-1));

    if (mBackground2->getPositionY() <= 0) {
        mBackground1->setPosition(mBackground2->getPosition());
    }

}
