#include "GameLayer.h"
#include "BGLayer.h"
#include "HeroSprite.h"
#include "Constant.h"
#include "HeroFollow.h"

bool GameLayer::init() {
    bool bRet = false;

    do {
        CC_BREAK_IF(!Layer::init());

		Size wsize = Director::getInstance()->getWinSize();
		Node *node = Node::create();
		node->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
		node->setPosition(wsize.width/2, wsize.height/2);
		PhysicsBody *body = PhysicsBody::createEdgeBox(wsize,
                PhysicsMaterial(10, 0.5, 0.5), 10);
        body->setCategoryBitmask(BITMASK_PHYS_WALL);
        body->setContactTestBitmask(BITMASK_PHYS_HERO);
		node->setPhysicsBody(body);
		addChild(node);

        /* hero layer */
        auto heroLayer = Layer::create();
        addChild(heroLayer);

        /* add BG */
        auto bg = BGLayer::create();
        addChild(bg);

        /* add hero */
        auto hero = HeroSprite::create();
        heroLayer->addChild(hero);

        bg->runAction(HeroFollow::create(hero));
        heroLayer->runAction(HeroFollow::create(hero));

        bRet = true;
    } while(0);

    return bRet;
}

void GameLayer::onEnter() {
    Layer::onEnter();
    auto touchListener = EventListenerTouchOneByOne::create();
    touchListener->onTouchBegan = CC_CALLBACK_2(GameLayer::onTouchBegan, this);
    touchListener->onTouchMoved = CC_CALLBACK_2(GameLayer::onTouchMoved, this);
    touchListener->onTouchEnded = CC_CALLBACK_2(GameLayer::onTouchEnded, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);
}

void GameLayer::onExit() {
    _eventDispatcher->removeEventListenersForTarget(this);
    Layer::onExit();
}

bool GameLayer::onTouchBegan(Touch *touch, Event *event) {
    return true;
}

void GameLayer::onTouchMoved(Touch *touch, Event *event) {
    Point p1 = touch->getStartLocation();
    Point p2 = touch->getLocation();
    auto body = PhysicsBody::createEdgeSegment(p1, p2,
            PhysicsMaterial(10, 0.5, 0.5), 15);
    body->setCategoryBitmask(BITMASK_PHYS_CORD);
    body->setContactTestBitmask(BITMASK_PHYS_HERO);
    auto node = Node::create();
    node->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
    node->setPhysicsBody(body);

    if (getChildByName("cord"))
        removeChildByName("cord");
    addChild(node, 0, "cord");
}

void GameLayer::onTouchEnded(Touch *touch, Event *event) {
}
