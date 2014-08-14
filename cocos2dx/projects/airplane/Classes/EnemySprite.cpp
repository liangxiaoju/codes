#include "EnemySprite.h"

using namespace CocosDenshion;

EnemySprite *EnemySprite::create(float multiple=1.0) {
    int type;
    float r = CCRANDOM_0_1()*3;

    if (r >=0 && r < 1.5) {
        type = TYPE_ENEMY1;
    } else if (r > 1.5 && r < 2.5) {
        type = TYPE_ENEMY2;
    } else {
        type = TYPE_ENEMY3;
    }

    return create(type, multiple);
}

EnemySprite *EnemySprite::create(int type, float multiple) {
    EnemySprite *enemy = new EnemySprite();
    if (enemy && enemy->initWithType(type, multiple)) {
        enemy->autorelease();
        return enemy;
    } else {
        delete enemy;
        return NULL;
    }
}

bool EnemySprite::initWithType(int type, float multiple) {
    bool bRet = false;
    char name[32];
    float duration;

    if (type == TYPE_ENEMY1) {
        initWithSpriteFrameName("enemy1.png");
        mLife = 1;
        duration = 3.5 / multiple;
        mCost = 1000;
    } else if (type == TYPE_ENEMY2) {
        initWithSpriteFrameName("enemy2.png");
        mLife = 3;
        duration = 4.5 / multiple;
        mCost = 3000;
    } else {
        snprintf(name, sizeof(name), "enemy3_n1.png");
        initWithSpriteFrameName(name);

        Animation *animation = Animation::create();
        animation->setDelayPerUnit(0.1f);
        animation->addSpriteFrame(SpriteFrameCache::getInstance()->getSpriteFrameByName("enemy3_n1.png"));
        animation->addSpriteFrame(SpriteFrameCache::getInstance()->getSpriteFrameByName("enemy3_n2.png"));
        Animate *animate = Animate::create(animation);
        runAction(RepeatForever::create(animate));
        mLife = 8;
        duration = 6.5 / multiple;
        mCost = 10000;
    }

    do {
        Size winSize = Director::getInstance()->getWinSize();
        Size enemySize = getContentSize();
        float x = CCRANDOM_0_1()*(winSize.width-enemySize.width)+enemySize.width/2;
        float y = winSize.height+enemySize.height/2;
        setPosition(x, y);

        MoveTo *moveTo = MoveTo::create(duration, Point(x, -enemySize.height/2));
        RemoveSelf *remove = RemoveSelf::create();
        Sequence *seq = Sequence::create(moveTo, remove, nullptr);
        runAction(seq);

        bRet = true;
    } while(0);

    mType = type;
    mBlowingUp = 0;

    return bRet;
}

void EnemySprite::blowUp() {
    if (mBlowingUp == 1)
        return;

    mBlowingUp = 1;

    stopAllActions();

    ScoreLayer::getInstance()->addScore(mCost);

    Animation *animation = Animation::create();
    animation->setDelayPerUnit(0.1f);

    if (mType == TYPE_ENEMY1) {
        animation->addSpriteFrame(SpriteFrameCache::getInstance()->getSpriteFrameByName("enemy1_down1.png"));
        animation->addSpriteFrame(SpriteFrameCache::getInstance()->getSpriteFrameByName("enemy1_down2.png"));
        animation->addSpriteFrame(SpriteFrameCache::getInstance()->getSpriteFrameByName("enemy1_down3.png"));
        animation->addSpriteFrame(SpriteFrameCache::getInstance()->getSpriteFrameByName("enemy1_down4.png"));
        SimpleAudioEngine::getInstance()->playEffect("sound/enemy1_down.mp3", false);
    } else if (mType == TYPE_ENEMY2) {
        animation->addSpriteFrame(SpriteFrameCache::getInstance()->getSpriteFrameByName("enemy2_down1.png"));
        animation->addSpriteFrame(SpriteFrameCache::getInstance()->getSpriteFrameByName("enemy2_down2.png"));
        animation->addSpriteFrame(SpriteFrameCache::getInstance()->getSpriteFrameByName("enemy2_down3.png"));
        animation->addSpriteFrame(SpriteFrameCache::getInstance()->getSpriteFrameByName("enemy2_down4.png"));
        SimpleAudioEngine::getInstance()->playEffect("sound/enemy2_down.mp3", false);
    } else {
        animation->addSpriteFrame(SpriteFrameCache::getInstance()->getSpriteFrameByName("enemy3_down1.png"));
        animation->addSpriteFrame(SpriteFrameCache::getInstance()->getSpriteFrameByName("enemy3_down2.png"));
        animation->addSpriteFrame(SpriteFrameCache::getInstance()->getSpriteFrameByName("enemy3_down3.png"));
        animation->addSpriteFrame(SpriteFrameCache::getInstance()->getSpriteFrameByName("enemy3_down4.png"));
        animation->addSpriteFrame(SpriteFrameCache::getInstance()->getSpriteFrameByName("enemy3_down5.png"));
        animation->addSpriteFrame(SpriteFrameCache::getInstance()->getSpriteFrameByName("enemy3_down6.png"));
        SimpleAudioEngine::getInstance()->playEffect("sound/enemy3_down.mp3", false);
    }

    Animate *animate = Animate::create(animation);
    RemoveSelf *remove = RemoveSelf::create();
    Sequence *seq = Sequence::create(animate, remove, nullptr);
    runAction(seq);
}

void EnemySprite::hit() {
    --mLife;

    if (mLife <= 0)
        return;

    if (mType == TYPE_ENEMY1)
        return;

    Animation *animation = Animation::create();
    animation->setDelayPerUnit(0.1f);

    if (mType == TYPE_ENEMY2) {
        animation->addSpriteFrame(SpriteFrameCache::getInstance()->getSpriteFrameByName("enemy2_hit.png"));
        animation->addSpriteFrame(SpriteFrameCache::getInstance()->getSpriteFrameByName("enemy2.png"));
    } else if (mType == TYPE_ENEMY3) {
        animation->addSpriteFrame(SpriteFrameCache::getInstance()->getSpriteFrameByName("enemy3_hit.png"));
        animation->addSpriteFrame(SpriteFrameCache::getInstance()->getSpriteFrameByName("enemy3_n1.png"));
    }
    Animate *animate = Animate::create(animation);
    runAction(animate);
}

bool EnemySprite::isAlive() {
    return (mLife > 0) ? true : false;
}

