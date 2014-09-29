#include "HeroSprite.h"
#include "Constant.h"
#include "GameOverScene.h"

bool HeroSprite::init() {
    bool bRet = false;

    do {
        CC_BREAK_IF(!Sprite::initWithFile("heroStand.png"));
        setColor(Color3B::RED);

        auto vsize = Director::getInstance()->getVisibleSize();
        auto heroSize = getContentSize();

        //setPosition(vsize.width/2, vsize.height/2);
        setPosition(vsize.width/2, heroSize.height/2);

        setScale(2.0);

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
        body->setVelocityLimit(500);
        setPhysicsBody(body);

        setState(STATE_IDLE);

        scheduleUpdate();

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
        if (contact.getContactData()->normal.y < 0) {
            setState(STATE_STAND);
            return true;
        } else
            return false;
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
        auto node = getNodeByBitmask(contact, BITMASK_PHYS_CORD);
        auto length= node->getContentSize().width;
        float rotation = node->getRotation();

        Vec2 v = Vec2(500 * (400-length)/100.0f, 0);
        v.rotate(Vec2::ZERO, CC_DEGREES_TO_RADIANS(90-rotation));

        getPhysicsBody()->setVelocity(v);

        setState(STATE_JUMPUP);
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

void HeroSprite::setState(int state) {
    int oldState = mState;
    mState = state;

    switch (state) {
        case STATE_IDLE:
        {
            log("STATE_IDLE");
            getPhysicsBody()->setGravityEnable(false);
            auto vsize = Director::getInstance()->getVisibleSize();
            auto jump = JumpBy::create(1, Vec2(0, 0), 400, 1);
            runAction(RepeatForever::create(jump));
            break;
        }
        case STATE_STAND:
        {
            if (oldState == STATE_IDLE) {
                getPhysicsBody()->setGravityEnable(true);
                stopAllActions();
            }
            log("STATE_STAND");
            stopAllActions();
            setTexture("heroStand.png");

            Animation *animation = Animation::create();
            animation->setDelayPerUnit(0.02f);
            for (int i=1; i <=17; i++) {
                char name[64];
                snprintf(name, sizeof(name), "effectSmokeLand_%04d.png", i);
                /*
                SpriteFrame *frame = SpriteFrameCache::getInstance()
                    ->getSpriteFrameByName(name);
                animation->addSpriteFrame(frame);
                */
                animation->addSpriteFrameWithFile(name);
            }
            Animate *animate = Animate::create(animation);
            Sequence *seq = Sequence::create(animate, RemoveSelf::create(), nullptr);

            //Sprite *smoke = Sprite::createWithSpriteFrameName("effectSmokeLand_0017.png");
            Sprite *smoke = Sprite::create("effectSmokeLand_0017.png");
            smoke->setScale(1.0);
            smoke->setPosition(getPosition() - Vec2(0, getContentSize().height/2+smoke->getContentSize().height/2));
            smoke->runAction(seq);
            getParent()->addChild(smoke);

            break;
        }
        case STATE_JUMPUP:
        {
            log("STATE_JUMPUP");
            setTexture("heroJumpUp.png");

            Animation *animation = Animation::create();
            animation->setDelayPerUnit(0.03f);
            for (int i=1; i <=22; i++) {
                char name[64];
                snprintf(name, sizeof(name), "effectSmokeRun_%04d.png", i);
                /*
                SpriteFrame *frame = SpriteFrameCache::getInstance()
                    ->getSpriteFrameByName(name);
                animation->addSpriteFrame(frame);
                */
                animation->addSpriteFrameWithFile(name);
            }
            Animate *animate = Animate::create(animation);
            Sequence *seq = Sequence::create(animate, RemoveSelf::create(), nullptr);

            Sprite *smoke = Sprite::create("effectSmokeRun_0001.png");
            smoke->setPosition(getContentSize().width/2, -smoke->getContentSize().height/2);
            smoke->runAction(seq);
            addChild(smoke);

            break;
        }
        case STATE_JUMPDOWN:
        {
            log("STATE_JUMPDOWN");
            setTexture("heroJumpDown.png");

            Animation *animation = Animation::create();
            animation->setDelayPerUnit(0.02f);
            char name[64];
            for (int i=1; i <=11; i++) {
                snprintf(name, sizeof(name), "heroRun_%04d.png", i);
                /*
                SpriteFrame *frame = SpriteFrameCache::getInstance()
                    ->getSpriteFrameByName(name);
                animation->addSpriteFrame(frame);
                */
                animation->addSpriteFrameWithFile(name);
            }
            Animate *animate = Animate::create(animation);
            Sequence *seq = Sequence::create(
                    animate,
                    DelayTime::create(0.05),
                    animate->reverse(),
                    DelayTime::create(0.05),
                    nullptr);
            runAction(seq);

            break;
        }
        default:
            break;
    }
}

void HeroSprite::update(float dt) {
    if (mState == STATE_JUMPUP) {
        Vec2 v = getPhysicsBody()->getVelocity();
        if (v.y < 0.001f) {
            setState(STATE_JUMPDOWN);
        }
    }

    Point point = getPosition();
    point = convertToWorldSpace(point);
    if (point.y < -100) {
        CallFunc *callback = CallFunc::create([]() {
            Director::getInstance()->replaceScene(GameOverScene::create());
        });

        auto seq = Sequence::create(DelayTime::create(0.5), callback, nullptr);
        runAction(seq);

		EventCustom event("EVENT_GameOver");
		_eventDispatcher->dispatchEvent(&event);
    }
}
