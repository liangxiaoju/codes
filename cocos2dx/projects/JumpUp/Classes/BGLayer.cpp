#include "BGLayer.h"
#include "Constant.h"

bool BGLayer::init() {
    bool bRet = false;

    do {
        CC_BREAK_IF(!Layer::init());

        auto vsize = Director::getInstance()->getVisibleSize();
        auto bg1 = Sprite::create("background.png");
        bg1->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
        bg1->setPosition(0, 0);
        addChild(bg1, 0, "background1");
        auto s = bg1->getContentSize();
        auto bg2 = Sprite::create("background.png");
        bg2->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
        bg2->setPosition(0, s.height);
        addChild(bg2, 0, "background2");

        scheduleUpdate();

        bRet = true;
    } while(0);

    return bRet;
}

void BGLayer::update(float dt) {
    Layer::update(dt);

    auto bg1 = getChildByName("background1");
    auto bg2 = getChildByName("background2");
    Point bg2_pos = convertToWorldSpace(bg2->getPosition());

    if (bg2_pos.y < 5) {
        auto s = bg1->getContentSize();
        auto p = convertToNodeSpace(Vec2(0, 0));
        bg1->setPosition(p);
        bg2->setPosition(p+Vec2(0, s.height));
    }
}

