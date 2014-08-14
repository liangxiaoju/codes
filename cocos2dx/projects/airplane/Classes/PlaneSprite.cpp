#include "PlaneSprite.h"

using namespace CocosDenshion;

PlaneSprite *PlaneSprite::create() {
    PlaneSprite *plane = new PlaneSprite();
    if (plane && plane->init()) {
        plane->autorelease();
        return plane;
    }
    CC_SAFE_DELETE(plane);
    return nullptr;
}

PlaneSprite *PlaneSprite::create(Point pos) {
    PlaneSprite *plane = create();
    if (plane)
        plane->setPosition(pos);
    return plane;
}

bool PlaneSprite::init() {
    bool bRet = false;

    do {
        initWithSpriteFrameName("hero1.png");
        setPosition(Vec2(Director::getInstance()->getWinSize().width/2,
                    getContentSize().height/2));

        mSuperPower = 0;

        bRet = true;
    } while (0);

    return bRet;
}

void PlaneSprite::blowUp() {
    SimpleAudioEngine::getInstance()->stopAllEffects();
    SimpleAudioEngine::getInstance()->playEffect("sound/game_over.mp3", false);

    stopAllActions();
    unscheduleAllSelectors();

    Animation *animation = Animation::create();
    animation->setDelayPerUnit(0.1f);
    animation->addSpriteFrame(
            SpriteFrameCache::getInstance()->getSpriteFrameByName("hero_blowup_n1.png"));
    animation->addSpriteFrame(
            SpriteFrameCache::getInstance()->getSpriteFrameByName("hero_blowup_n2.png"));
    animation->addSpriteFrame(
            SpriteFrameCache::getInstance()->getSpriteFrameByName("hero_blowup_n3.png"));
    animation->addSpriteFrame(
            SpriteFrameCache::getInstance()->getSpriteFrameByName("hero_blowup_n4.png"));
    Animate *animate = Animate::create(animation);

    RemoveSelf *remove = RemoveSelf::create();
    Sequence *seq = Sequence::create(animate, remove, nullptr);

    runAction(seq);
}

void PlaneSprite::startShoot() {
    schedule(schedule_selector(PlaneSprite::shootOnce), 0.12, kRepeatForever, 0);
}

void PlaneSprite::shootOnce(float dt) {
    BulletSprite *bullet1, *bullet2;

    SimpleAudioEngine::getInstance()->playEffect("sound/bullet.mp3", false, 1.0, 0.0, 0.6);

    if (mSuperPower == 0) {
        bullet1 = BulletSprite::create(1);
        bullet1->setPosition(this->getPosition() + Vec2(0, 10));
        GameLayer::getInstance()->addChild(bullet1, 0, "bullet");
        bullet1->start();
    } else {
        bullet1 = BulletSprite::create(2);
        bullet2 = BulletSprite::create(2);
        bullet1->setPosition(this->getPosition() + Vec2(-5, 10));
        bullet2->setPosition(this->getPosition() + Vec2(5, 10));
        GameLayer::getInstance()->addChild(bullet1, 0, "bullet");
        GameLayer::getInstance()->addChild(bullet2, 0, "bullet");
        bullet1->start();
        bullet2->start();
    }
}

void PlaneSprite::superPower() {
    mSuperPower = 1;
    DelayTime *delay = DelayTime::create(15);
    CallFunc *callback = CallFunc::create([&]() {
        this->mSuperPower = 0;
    });
    Sequence *seq = Sequence::create(delay, callback, nullptr);
    runAction(seq);

    SimpleAudioEngine::getInstance()->playEffect("sound/get_double_laser.mp3", false);
}

void PlaneSprite::onEnterTransitionDidFinish() {
    Sprite::onEnterTransitionDidFinish();

    Blink *blink = Blink::create(1, 3);
    Animation *animation = Animation::create();
    animation->setDelayPerUnit(0.1f);
    animation->addSpriteFrame(
            SpriteFrameCache::getInstance()->getSpriteFrameByName("hero1.png"));
    animation->addSpriteFrame(
            SpriteFrameCache::getInstance()->getSpriteFrameByName("hero2.png"));
    Animate *animate = Animate::create(animation);

    runAction(blink);
    runAction(RepeatForever::create(animate));

    auto keyListener = EventListenerKeyboard::create();
    keyListener->onKeyPressed = CC_CALLBACK_2(PlaneSprite::onKeyPressed, this);
    keyListener->onKeyReleased = CC_CALLBACK_2(PlaneSprite::onKeyReleased, this);
    Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(keyListener, this);

    auto touchListener = EventListenerTouchOneByOne::create();
    touchListener->onTouchBegan = CC_CALLBACK_2(PlaneSprite::onTouchBegan, this);
    touchListener->onTouchMoved = CC_CALLBACK_2(PlaneSprite::onTouchMoved, this);
    touchListener->onTouchEnded = CC_CALLBACK_2(PlaneSprite::onTouchEnded, this);
    touchListener->onTouchCancelled = CC_CALLBACK_2(PlaneSprite::onTouchCancelled, this);
    Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(touchListener, this);

    startShoot();
}

void PlaneSprite::moveTo(float duration, Point position, int tag) {
    FiniteTimeAction *action;

    if (duration == 0.0) {
        action = Place::create(position);
    } else {
        action = MoveTo::create(duration, position);
    }

    action->setTag(tag);
    runAction(action);
}

void PlaneSprite::stop(int tags) {
    stopActionByTag(tags);
}

void PlaneSprite::onKeyPressed(EventKeyboard::KeyCode keyCode, Event *unused) {
    Point pos = this->getPosition();
    Size size = Director::getInstance()->getWinSize();
    const int pbs = 200;
    switch (keyCode) {
    case EventKeyboard::KeyCode::KEY_LEFT_ARROW:
        moveTo(pos.x/pbs, Vec2(0, pos.y), LEFT);
        break;
    case EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
        moveTo((size.width-pos.x)/pbs, Vec2(size.width, pos.y), RIGHT);
        break;
    case EventKeyboard::KeyCode::KEY_UP_ARROW:
        moveTo((size.height-pos.y)/pbs, Vec2(pos.x, size.height), UP);
        break;
    case EventKeyboard::KeyCode::KEY_DOWN_ARROW:
        moveTo((pos.y)/pbs, Vec2(pos.x, 0), DOWN);
        break;
    default:
        break;
    }
}

void PlaneSprite::onKeyReleased(EventKeyboard::KeyCode keyCode, Event *unused) {
    switch (keyCode) {
    case EventKeyboard::KeyCode::KEY_LEFT_ARROW:
        stop(LEFT);
        break;
    case EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
        stop(RIGHT);
        break;
    case EventKeyboard::KeyCode::KEY_UP_ARROW:
        stop(UP);
        break;
    case EventKeyboard::KeyCode::KEY_DOWN_ARROW:
        stop(DOWN);
        break;
    default:
        break;
    }
}

bool PlaneSprite::onTouchBegan(Touch *touch, Event *event) {
    Point touchPos = touch->getLocation();
    Rect planeRect = this->getBoundingBox();
    planeRect.origin -= Vec2(5, 5);
    planeRect.size = planeRect.size + Size(10, 10);

    if (planeRect.containsPoint(touchPos))
        return true;

    return false;
}

void PlaneSprite::onTouchMoved(Touch *touch, Event *event) {
    Point pos = touch->getLocation();
    moveTo(0, pos, 0);
}

void PlaneSprite::onTouchEnded(Touch *touch, Event *event) {
}

void PlaneSprite::onTouchCancelled(Touch *touch, Event *event) {
}
