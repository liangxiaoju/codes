#include "HeroSprite.h"
#include "Constant.h"
#include "GameOverScene.h"
#include "BackgroundLayer.h"

bool HeroSprite::init() {
	bool bRet = false;

	do {
		CC_BREAK_IF(!Sprite::initWithSpriteFrameName("heroStand_0001.png"));

		Size heroSize = this->getContentSize();
		PhysicsBody *body = PhysicsBody::createBox(heroSize-Size(0,16), PhysicsMaterial(0, 0, 0));
		body->setLinearDamping(0.0f);
		body->setDynamic(true);
		body->setGravityEnable(true);
		body->setTag(TAG_HERO_PHYS_BODY);
		body->setCategoryBitmask(1<<2);
		body->setContactTestBitmask(1<<0 | 1<<1);
		body->setMass(50);
		body->setRotationEnable(false);

		this->setPhysicsBody(body);

		mState = STATE_IDLE;
        mRunAnimate = NULL;
        mSmokeRunAnimate = NULL;

		bRet = true;
	} while(0);

	return bRet;
}

void HeroSprite::run() {
    if (mRunAnimate == NULL) {
        Animation *animation = Animation::create();
        animation->setDelayPerUnit(0.03f);
        char name[64];
        for (int i=1; i <=11; i++) {
            snprintf(name, sizeof(name), "heroRun_%04d.png", i);
    		SpriteFrame *frame = SpriteFrameCache::getInstance()
				->getSpriteFrameByName(name);
            animation->addSpriteFrame(frame);
        }
        Animate *animate = Animate::create(animation);
        Sequence *seq = Sequence::create(
				animate,
				DelayTime::create(0.1),
				animate->reverse(),
				DelayTime::create(0.1),
				nullptr);
        RepeatForever *repeat = RepeatForever::create(seq);
        mRunAnimate = repeat;
        mRunAnimate->retain();
    }
	runAction(mRunAnimate);

    if (mSmokeRunAnimate == NULL) {
        Animation *animation = Animation::create();
        animation->setDelayPerUnit(0.03f);
        for (int i=1; i <=22; i++) {
            char name[64];
            snprintf(name, sizeof(name), "effectSmokeRun_%04d.png", i);
            SpriteFrame *frame = SpriteFrameCache::getInstance()
                ->getSpriteFrameByName(name);
            animation->addSpriteFrame(frame);
        }
        Animate *animate = Animate::create(animation);
        Sequence *seq = Sequence::create(animate, RemoveSelf::create(), nullptr);
        mSmokeRunAnimate = seq;
        mSmokeRunAnimate->retain();
    }

    CallFunc *callback = CallFunc::create([&]() {
            Sprite *smoke = Sprite::createWithSpriteFrameName("effectSmokeRun_0022.png");
            smoke->setOpacity(160);
            smoke->setScale(0.8);
            smoke->setAnchorPoint(Vec2(0, 0));
            smoke->setPosition(getPosition() - getContentSize()/2);
            smoke->runAction(mSmokeRunAnimate->clone());
            auto move = MoveBy::create(1, Vec2(-300, 0));
            smoke->runAction(move);
            getParent()->addChild(smoke, 0, "smokeRun");
            });
    auto seq = Sequence::create(callback, DelayTime::create(0.30f), nullptr);
    runAction(RepeatForever::create(seq));

	mState = STATE_RUN;
}

void HeroSprite::jump() {
    Vect pulse;

	if (mState == STATE_JUMP2)
		return;
	else if (mState == STATE_IDLE || mState == STATE_DEAD)
		return;
    else if (mState == STATE_JUMP1) {
        mState = STATE_JUMP2;
        pulse = Vect(0, 6000);
    } else {
        mState = STATE_JUMP1;
        pulse = Vect(0, 8000);
    }

	setSpriteFrame("heroJump_0001.png");

    stopAllActions();

    getPhysicsBody()->applyImpulse(pulse);

	auto delay1 = DelayTime::create(0.06f);
	auto callback1 = CallFunc::create([&]() {
			getPhysicsBody()->applyForce(Vect(0, 1500));
			});
	auto delay2 = DelayTime::create(0.8f);
	auto callback2 = CallFunc::create([&]() {
			getPhysicsBody()->resetForces();
			});
	auto seq = Sequence::create(delay1, callback1, delay2, callback2, nullptr);
	seq->setTag(10);
	runAction(seq);

	getPhysicsBody()->setContactTestBitmask((1<<0) | (1<<1));
}

void HeroSprite::stand() {
	mState = STATE_STAND;

    if (mSmokeStandAnimate == NULL) {
        Animation *animation = Animation::create();
        animation->setDelayPerUnit(0.02f);
        for (int i=1; i <=17; i++) {
            char name[64];
            snprintf(name, sizeof(name), "effectSmokeLand_%04d.png", i);
            SpriteFrame *frame = SpriteFrameCache::getInstance()
                ->getSpriteFrameByName(name);
            animation->addSpriteFrame(frame);
        }
        Animate *animate = Animate::create(animation);
        Sequence *seq = Sequence::create(animate, RemoveSelf::create(), nullptr);
        mSmokeStandAnimate = seq;
        mSmokeStandAnimate->retain();
    }

    Sprite *smoke = Sprite::createWithSpriteFrameName("effectSmokeLand_0017.png");
    smoke->setScale(0.6);
    smoke->setPosition(getPosition() - Vec2(0, getContentSize().height/2+smoke->getContentSize().height/2));
    auto seq = Sequence::create(mSmokeStandAnimate, RemoveSelf::create(), nullptr);
    smoke->runAction(seq);
    getParent()->addChild(smoke);
}

void HeroSprite::dead() {
	mState = STATE_DEAD;
	stopAllActions();
	getPhysicsBody()->setVelocity(Vec2(0, 0));
	getPhysicsBody()->resetForces();
	getPhysicsBody()->applyImpulse(Vec2(-2000, 5000));
}

void HeroSprite::onEnterTransitionDidFinish() {
	Sprite::onEnterTransitionDidFinish();

	auto eventDispatcher = Director::getInstance()->getEventDispatcher();

    auto keyListener = EventListenerKeyboard::create();
    keyListener->onKeyPressed = CC_CALLBACK_2(HeroSprite::onKeyPressed, this);
    keyListener->onKeyReleased = CC_CALLBACK_2(HeroSprite::onKeyReleased, this);
    eventDispatcher->addEventListenerWithSceneGraphPriority(keyListener, this);

    auto touchListener = EventListenerTouchOneByOne::create();
    touchListener->onTouchBegan = CC_CALLBACK_2(HeroSprite::onTouchBegan, this);
    touchListener->onTouchMoved = CC_CALLBACK_2(HeroSprite::onTouchMoved, this);
    touchListener->onTouchEnded = CC_CALLBACK_2(HeroSprite::onTouchEnded, this);
    eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);

	auto contactListener = EventListenerPhysicsContact::create();
	contactListener->onContactBegin = CC_CALLBACK_1(HeroSprite::onContactBegin, this);
	contactListener->onContactPreSolve = CC_CALLBACK_2(HeroSprite::onContactPreSolve, this);
	contactListener->onContactPostSolve = CC_CALLBACK_2(HeroSprite::onContactPostSolve, this);
	eventDispatcher->addEventListenerWithSceneGraphPriority(contactListener, this);
}

void HeroSprite::onKeyPressed(EventKeyboard::KeyCode keyCode, Event* unused_event) {
    switch (keyCode) {
    case EventKeyboard::KeyCode::KEY_UP_ARROW:
		jump();
		break;
	default:
		break;
	}
}

void HeroSprite::onKeyReleased(EventKeyboard::KeyCode keyCode, Event* unused_event) {
    switch (keyCode) {
    case EventKeyboard::KeyCode::KEY_UP_ARROW:
		stopActionByTag(10);
    	getPhysicsBody()->resetForces();
		break;
	default:
		break;
	}
}

bool HeroSprite::onTouchBegan(Touch *touch, Event *event) {
	jump();
    return true;
}

void HeroSprite::onTouchMoved(Touch *touch, Event *event) {
}

void HeroSprite::onTouchEnded(Touch *touch, Event *event) {
	stopActionByTag(10);
	getPhysicsBody()->resetForces();
}

bool hitTest(PhysicsContact& contact, int mask) {
    PhysicsBody* a = contact.getShapeA()->getBody();
    PhysicsBody* b = contact.getShapeB()->getBody();
    int tagA = a->getTag();
    int tagB = b->getTag();
    int tag = tagA | tagB;

    return (tag & mask) == mask;
}

Node *getNodeByTag(PhysicsContact& contact, int tag) {
    PhysicsBody* a = contact.getShapeA()->getBody();
    PhysicsBody* b = contact.getShapeB()->getBody();
    int tagA = a->getTag();
    int tagB = b->getTag();

	Node *node;

	if (tagA == tag)
		node = a->getNode();
	else if (tagB == tag)
		node = b->getNode();
	else
		node = NULL;

	return node;
}

bool HeroSprite::onContactBegin(PhysicsContact& contact) {

	if (mState == STATE_DEAD) {
		return false;
	}

    if (hitTest(contact, TAG_HERO_PHYS_BODY|TAG_GROUND1_PHYS_BODY)) {

		Vec2 v = getPhysicsBody()->getVelocity();
		getPhysicsBody()->setVelocity(Vec2(0, 0));
		getPhysicsBody()->resetForces();
		getPhysicsBody()->setContactTestBitmask(1<<1);

        if ((mState == STATE_JUMP1)
                || (mState == STATE_JUMP2)
                || (mState == STATE_IDLE)) {

			if (v.y < -200)
				stand();
			else
				mState = STATE_STAND;

			CallFunc *callback = CallFunc::create([&]() {
				run();
			});
			auto delay = DelayTime::create(0.05f);
			auto seq = Sequence::create(delay, callback, nullptr);
			runAction(seq);
        }
    }

    if (hitTest(contact, TAG_HERO_PHYS_BODY|TAG_BARRIER_PHYS_BODY)) {
		BackgroundLayer * bg = (BackgroundLayer *)(getScene()
				->getChildByName("gamelayer")
				->getChildByName("background"));
		bg->stopMove();

		auto scale1 = ScaleTo::create(0.1, 0.9, 0.9);
		auto scale2 = ScaleTo::create(0.1, 1.0, 1.0);
		auto seq1 = Sequence::create(scale1, scale2, nullptr);
		getNodeByTag(contact, TAG_BARRIER_PHYS_BODY)->runAction(seq1);

		getPhysicsBody()->setContactTestBitmask(1<<1 | 1<<0);
		dead();

		CallFunc *callback = CallFunc::create([]() {
			Director::getInstance()->replaceScene(GameOverScene::create());
		});
		auto seq = Sequence::create(DelayTime::create(1), callback, nullptr);
		runAction(seq);

		EventCustom event("EVENT_GameOver");
		_eventDispatcher->dispatchEvent(&event);
    }

	return true;
}

bool HeroSprite::onContactPreSolve(PhysicsContact& contact, PhysicsContactPreSolve& solve) {
    return true;
}

void HeroSprite::onContactPostSolve(PhysicsContact& contact, const PhysicsContactPostSolve& solve) {
}

