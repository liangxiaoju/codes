#include "BulletSprite.h"

BulletSprite* BulletSprite::create(int type) {
    BulletSprite *pRet = new BulletSprite();
    if (pRet && pRet->initWithType(type)) {
        pRet->autorelease();
        return pRet;
    } else {
        delete pRet;
        pRet = NULL;
        return NULL;
    }
}

bool BulletSprite::initWithType(int type) {

    if (type == 1) {
        Sprite::initWithSpriteFrameName("bullet1.png");
    } else if (type == 2) {
        Sprite::initWithSpriteFrameName("bullet2.png");
    }

    return true;
}

void BulletSprite::start() {
    Point pos = getPosition();
    Size size = Director::getInstance()->getWinSize();
    float duration = (size.height - pos.y)/1200;

    if (duration <= 0) {
        duration = 0;
    }

    MoveTo *moveTo = MoveTo::create(duration, Vec2(pos.x, size.height));
    RemoveSelf *remove = RemoveSelf::create();
    Sequence *seq = Sequence::create(moveTo, remove, nullptr);
    runAction(seq);
}

void BulletSprite::blowUp() {
    stopAllActions();
    runAction(RemoveSelf::create());
}

