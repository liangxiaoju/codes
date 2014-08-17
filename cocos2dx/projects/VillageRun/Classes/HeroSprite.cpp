#include "HeroSprite.h"

bool HeroSprite::init() {
	bool bRet = false;

	do {
		CC_BREAK_IF(!Sprite::initWithFile("ui/heroSheet/heroStand_0001.png"));

		Size heroSize = this->getContentSize();
		PhysicsBody *body = PhysicsBody::createCircle(heroSize.height/2, PhysicsMaterial(0, 0, 0));
		body->setLinearDamping(0.0f);
		body->setDynamic(true);
		body->setGravityEnable(true);
		body->setTag(101);
		body->setContactTestBitmask(0xFFFFFFFF);
		//body->setVelocityLimit(800);
		body->setMass(20);
		body->setRotationEnable(false);

		this->setPhysicsBody(body);

		stand();

		mState = STATE_IDLE;

		bRet = true;
	} while(0);

	return bRet;
}

void HeroSprite::run() {
    Animation *animation = Animation::create();
    animation->setDelayPerUnit(0.05f);
	for (int i=1; i <=11; i++) {
		char name[64];
		snprintf(name, sizeof(name), "ui/heroSheet/heroRun_%04d.png", i);
    	animation->addSpriteFrameWithFile(name);
	}
	Animate *animate = Animate::create(animation);
	runAction(RepeatForever::create(animate));
	mState = STATE_RUN;
}

void HeroSprite::jump() {
	if (mState == STATE_JUMP)
		return;

	stopAllActions();
	setTexture("ui/heroSheet/heroJump_0001.png");
	//getPhysicsBody()->setVelocity(Vec2(0, 900));
	//getPhysicsBody()->applyForce(Vect(0, 20*1200*2));
	getPhysicsBody()->applyImpulse(Vect(0, 13000));
	
	CallFunc *callback = CallFunc::create([&]() {
			getPhysicsBody()->applyImpulse(Vect(0, 12000));
		});
	DelayTime *delay = DelayTime::create(0.15f);
	auto seq = Sequence::create(delay, callback, nullptr);
	auto repeat = Repeat::create(seq, 3);
	seq->setTag(1);
	runAction(seq);
	mState = STATE_JUMP;
}

void HeroSprite::stand() {
	setTexture("ui/heroSheet/heroStand_0001.png");
	mState = STATE_STAND;
}

void HeroSprite::onEnterTransitionDidFinish() {
	Sprite::onEnterTransitionDidFinish();
	run();

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
		stopActionByTag(1);
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
	stopActionByTag(1);
}

bool HeroSprite::onContactBegin(PhysicsContact& contact) {
    PhysicsBody* a = contact.getShapeA()->getBody();
    PhysicsBody* b = contact.getShapeB()->getBody();
	//CCLog("%s\n", __func__);

	if (mState == STATE_JUMP) {
		stand();
		run();
	}

	return true;
}

