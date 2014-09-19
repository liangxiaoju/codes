#include "HeroSprite.h"
#include "Constant.h"

bool HeroSprite::init() {
    bool bRet = false;

    do {
        CC_BREAK_IF(!Sprite::initWithFile("CloseNormal.png"));
        setColor(Color3B::BLUE);

        auto vsize = Director::getInstance()->getVisibleSize();

        setPosition(vsize.width/2, vsize.height/2);

        auto heroSize = getContentSize();
        auto body = PhysicsBody::createCircle((heroSize.width+heroSize.height)/4);
        body->setTag(TAG_PHYS_BODY_HERO);
        body->setMass(1);
        body->setRotationEnable(false);
        body->setCategoryBitmask(BITMASK_PHYS_HERO);
        body->setContactTestBitmask(
                BITMASK_PHYS_WALL |
                BITMASK_PHYS_BARRIER |
                BITMASK_PHYS_ENEMY |
                BITMASK_PHYS_GROUND |
                BITMASK_PHYS_CORD);
        body->setVelocityLimit(300);
        setPhysicsBody(body);

        bRet = true;
    } while(0);

    return bRet;
}

void HeroSprite::onEnter() {
    Sprite::onEnter();

	auto contactListener = EventListenerPhysicsContact::create();
	contactListener->onContactBegin = CC_CALLBACK_1(HeroSprite::onContactBegin, this);
	contactListener->onContactPreSolve = CC_CALLBACK_2(HeroSprite::onContactPreSolve, this);
	contactListener->onContactPostSolve = CC_CALLBACK_2(HeroSprite::onContactPostSolve, this);
    contactListener->onContactSeperate = CC_CALLBACK_1(HeroSprite::onContactSeperate, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(contactListener, this);
}

void HeroSprite::onExit() {
    _eventDispatcher->removeEventListenersForTarget(this);

    Sprite::onExit();
}

bool hitTest(PhysicsContact& contact, int a, int b) {
    int maskA = contact.getShapeA()->getCategoryBitmask();
    int maskB = contact.getShapeB()->getCategoryBitmask();
    int mask = maskA | maskB;

    return (mask & (a|b)) == (a|b);
}

Node *getNodeByBitmask(PhysicsContact& contact, int mask) {
    PhysicsBody* a = contact.getShapeA()->getBody();
    PhysicsBody* b = contact.getShapeB()->getBody();
    int maskA = a->getCategoryBitmask();
    int maskB = b->getCategoryBitmask();

	Node *node = NULL;

	if (maskA == mask)
		node = a->getNode();
	else if (maskB == mask)
		node = b->getNode();

	return node;
}

bool HeroSprite::onContactBegin(PhysicsContact& contact) {
    if (hitTest(contact, BITMASK_PHYS_HERO, BITMASK_PHYS_CORD)) {
        return contact.getContactData()->normal.y < 0;
    } else {
        return true;
    }
}

bool HeroSprite::onContactPreSolve(PhysicsContact& contact, PhysicsContactPreSolve& solve) {

    solve.setRestitution(0.8);

    return true;
}

void HeroSprite::onContactPostSolve(PhysicsContact& contact, const PhysicsContactPostSolve& solve) {
    if (hitTest(contact, BITMASK_PHYS_HERO, BITMASK_PHYS_CORD)) {
        Vec2 v = getPhysicsBody()->getVelocity();
        log("%f %f", v.x, v.y);
        getPhysicsBody()->resetForces();
        getPhysicsBody()->setVelocity(Vec2(0,0));
        //getPhysicsBody()->applyImpulse(v/0.8);
        getPhysicsBody()->setVelocity(v*2);
    }
}

/* cocos2dx's event callback is async in multithreading context */
void HeroSprite::onContactSeperate(PhysicsContact& contact) {
    if (hitTest(contact, BITMASK_PHYS_HERO, BITMASK_PHYS_CORD)) {
        /* competition */
        auto node = getNodeByBitmask(contact, BITMASK_PHYS_CORD);
        if (node && node->isVisible()) {
            node->setVisible(false);
            node->removeFromParent();
        }
    }
}
