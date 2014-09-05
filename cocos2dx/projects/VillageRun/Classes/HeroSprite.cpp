#include "HeroSprite.h"
#include "Constant.h"
#include "GameOverScene.h"

bool HeroSprite::init() {
	bool bRet = false;

	do {
		CC_BREAK_IF(!Sprite::initWithSpriteFrameName("heroStand_0001.png"));

		Size heroSize = this->getContentSize();
		//PhysicsBody *body = PhysicsBody::createCircle(heroSize.height/2-8, PhysicsMaterial(0, 0, 0));
		PhysicsBody *body = PhysicsBody::createBox(heroSize-Size(0,16), PhysicsMaterial(0, 0, 0));
		body->setLinearDamping(0.0f);
		body->setDynamic(true);
		body->setGravityEnable(true);
		body->setTag(TAG_HERO_PHYS_BODY);
		body->setContactTestBitmask(0xFFFFFFFF);
		//body->setVelocityLimit(800);
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
		//std::vector<int> array = {1, 2, 3, 5, 7, 9, 10, 11};
        //for (int i : array) {
        for (int i=1; i <=11; i++) {
            snprintf(name, sizeof(name), "heroRun_%04d.png", i);
    		SpriteFrame *frame = SpriteFrameCache::getInstance()
				->getSpriteFrameByName(name);
            animation->addSpriteFrame(frame);
        }
        Animate *animate = Animate::create(animation);
        Sequence *seq = Sequence::create(animate, animate->reverse(), nullptr);
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
            smoke->setScale(0.6);
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
    else if (mState == STATE_JUMP1) {
        mState = STATE_JUMP2;
        pulse = Vect(0, 7500);
    } else {
        mState = STATE_JUMP1;
        pulse = Vect(0, 8500);
    }

	setSpriteFrame("heroJump_0001.png");

    stopAllActions();

    getPhysicsBody()->applyImpulse(pulse);

	auto delay1 = DelayTime::create(0.06f);
	auto callback1 = CallFunc::create([&]() {
			getPhysicsBody()->applyForce(Vect(0, 1500));
			CCLog("applyForce");
			});
	auto delay2 = DelayTime::create(0.8f);
	auto callback2 = CallFunc::create([&]() {
			getPhysicsBody()->resetForces();
			CCLog("resetForce");
			});
	auto seq = Sequence::create(delay1, callback1, delay2, callback2, nullptr);
	seq->setTag(10);
	runAction(seq);
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
		CCLog("onKeyReleased: resetForce");
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
	CCLog("onTouchEnded: resetForce");
}

bool hitTest(PhysicsContact& contact, int mask) {
    PhysicsBody* a = contact.getShapeA()->getBody();
    PhysicsBody* b = contact.getShapeB()->getBody();
    int tagA = a->getTag();
    int tagB = b->getTag();
    int tag = tagA | tagB;

    return (tag & mask) == mask;
}

bool HeroSprite::onContactBegin(PhysicsContact& contact) {
	return true;
}

bool HeroSprite::onContactPreSolve(PhysicsContact& contact, PhysicsContactPreSolve& solve) {

    if (hitTest(contact, TAG_HERO_PHYS_BODY|TAG_GROUND1_PHYS_BODY)) {

        solve.setSurfaceVelocity(Vec2(0, 0));

        if ((mState == STATE_JUMP1)
                || (mState == STATE_JUMP2)
                || (mState == STATE_IDLE)) {

			stand();
			CallFunc *callback = CallFunc::create([&]() {
				run();
			});
			auto delay = DelayTime::create(0.01f);
			auto seq = Sequence::create(delay, callback, nullptr);
			runAction(seq);
        }
    }

    if (hitTest(contact, TAG_HERO_PHYS_BODY|TAG_BARRIER_PHYS_BODY)) {
        Director::getInstance()->replaceScene(GameOverScene::create());
    }

    return true;
}

