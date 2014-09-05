#include "UFOSprite.h"

UFOSprite* UFOSprite::create(int type) {
    UFOSprite *pRet = new UFOSprite();
    if (pRet && pRet->initWithType(type)) {
        pRet->autorelease();
        return pRet;
    } else {
        delete pRet;
        pRet = NULL;
        return NULL;
    }
}

bool UFOSprite::initWithType(int type) {
    mType = type;

    if (type == TYPE_UFO1) {
        initWithSpriteFrameName("ufo1.png");
    } else if (type == TYPE_UFO2) {
        initWithSpriteFrameName("ufo2.png");
    } else if (type == TYPE_UFO3) {
        //initWithSpriteFrameName("ufo3.png");
        initWithFile("ui/ufo3.png");
    }

    Size winSize = Director::getInstance()->getWinSize();
    Size s = getContentSize();
    float x = CCRANDOM_0_1()*(winSize.width-s.width)+s.width/2;
    float y = winSize.height+s.height/2;
    setPosition(x, y);

    auto m1 = MoveBy::create(0.4, Point(0, -150));
    auto m2 = MoveBy::create(0.2, Point(0, 100));
    auto m3 = MoveBy::create(0.8, Point(0, -winSize.height));
    auto m4 = RemoveSelf::create();
    Sequence *seq = Sequence::create(m1, m2, m3, m4, nullptr);
    runAction(seq);

    return true;
}

int UFOSprite::getType() {
    return mType;
}
