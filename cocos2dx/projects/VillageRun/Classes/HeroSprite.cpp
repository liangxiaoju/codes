#include "HeroSprite.h"
#include "Constant.h"
#include "GameOverScene.h"

bool HeroSprite::init() {
	bool bRet = false;

	do {
		CC_BREAK_IF(!Sprite::initWithFile("ui/heroSheet/heroStand_0001.png"));

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

		stand();

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
        animation->setDelayPerUnit(0.05f);
        for (int i=1; i <=11; i++) {
            char name[64];
            snprintf(name, sizeof(name), "ui/heroSheet/heroRun_%04d.png", i);
            animation->addSpriteFrameWithFile(name);
        }
        Animate *animate = Animate::create(animation);
        RepeatForever *repeat = RepeatForever::create(animate);
        mRunAnimate = repeat;
        mRunAnimate->retain();
    }
	runAction(mRunAnimate);
/*
    if (mSmokeRunAnimate == NULL) {
        Animation *animation = Animation::create();
        animation->setDelayPerUnit(0.02f);
        for (int i=1; i <=22; i++) {
            char name[64];
            snprintf(name, sizeof(name), "ui/spriteSheet/effectSmokeRun_%04d.png", i);
            animation->addSpriteFrameWithFile(name);
        }
        Animate *animate = Animate::create(animation);
        RepeatForever *repeat = RepeatForever::create(animate);
        mSmokeRunAnimate = repeat;
        mSmokeRunAnimate->retain();
    }
    Sprite *smoke = Sprite::create("ui/spriteSheet/effectSmokeRun_0001.png");
    smoke->setScale(0.5);
    smoke->setAnchorPoint(Vec2(0, 0));
    smoke->setPosition(0, 0);
    smoke->runAction(mSmokeRunAnimate);
    addChild(smoke, 0, "smokeRun");
*/

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
        //removeChildByName("smokeRun");
    }

    stopAllActions();
	setTexture("ui/heroSheet/heroJump_0001.png");
	
    getPhysicsBody()->applyImpulse(pulse);
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
}

bool HeroSprite::onContactBegin(PhysicsContact& contact) {
    PhysicsBody* a = contact.getShapeA()->getBody();
    PhysicsBody* b = contact.getShapeB()->getBody();
    int tagA = a->getTag();
    int tagB = b->getTag();

    if (((tagA==TAG_HERO_PHYS_BODY) && (tagB==TAG_GROUND1_PHYS_BODY))
        || ((tagA==TAG_GROUND1_PHYS_BODY) && (tagB==TAG_HERO_PHYS_BODY))) {
        if ((mState == STATE_JUMP1) || (mState == STATE_JUMP2)) {
            stand();
            run();
        }
    }

    if (((tagA == TAG_HERO_PHYS_BODY) && (tagB == TAG_BARRIER_PHYS_BODY))
        || ((tagA == TAG_BARRIER_PHYS_BODY) && (tagB == TAG_HERO_PHYS_BODY))) {

        CCLog("Game Over!\n");
        Director::getInstance()->replaceScene(GameOverScene::create());
    }

	return true;
}

