#include "WelcomeScene.h"
#include "SimpleAudioEngine.h"

using namespace CocosDenshion;

bool WelcomeScene::init() {
    bool bRet = false;

    do {
        CC_BREAK_IF(!Scene::init());
        SpriteFrameCache::getInstance()->addSpriteFramesWithFile(
                "ui/other.plist");
        SpriteFrameCache::getInstance()->addSpriteFramesWithFile(
                "ui/fish.plist");

        mLayer = WelcomeLayer::create();
        CC_BREAK_IF(!mLayer);

        addChild(mLayer);

        bRet = true;
    } while(0);

    return bRet;
}

bool WelcomeLayer::init() {
    bool bRet = false;

    do {
        CC_BREAK_IF(!Layer::init());

        mBg = Sprite::createWithSpriteFrameName("bg.png");
        mBg->setAnchorPoint(Vec2(0, 0));
        mBg->setPosition(0, 0);
        addChild(mBg);

//        Sprite *sprite = Sprite::createWithSpriteFrameName("game_loading1.png");
        Sprite *sprite = Sprite::create();
        Animation *animation = Animation::create();
        animation->setDelayPerUnit(0.3);
        animation->addSpriteFrame(
            SpriteFrameCache::getInstance()->getSpriteFrameByName("game_loading1.png"));
        animation->addSpriteFrame(
            SpriteFrameCache::getInstance()->getSpriteFrameByName("game_loading2.png"));
        animation->addSpriteFrame(
            SpriteFrameCache::getInstance()->getSpriteFrameByName("game_loading3.png"));
        animation->addSpriteFrame(
            SpriteFrameCache::getInstance()->getSpriteFrameByName("game_loading4.png"));
        Animate *animate = Animate::create(animation);
        Repeat *repeat = Repeat::create(animate, 2);
        CallFunc *callback = CallFunc::create([]() {
                    Director::getInstance()->replaceScene(GameScene::create());
                });
        Sequence *seq = Sequence::create(repeat, callback, nullptr);
        Size winSize = Director::getInstance()->getWinSize();
        sprite->setPosition(winSize.width/2, winSize.height/2);
        sprite->runAction(seq);
        addChild(sprite);

        SimpleAudioEngine::getInstance()->preloadBackgroundMusic("sound/game_music.mp3");
        SimpleAudioEngine::getInstance()->preloadEffect("sound/bullet.mp3");
        SimpleAudioEngine::getInstance()->preloadEffect("sound/enemy1_down.mp3");
        SimpleAudioEngine::getInstance()->preloadEffect("sound/enemy2_down.mp3");
        SimpleAudioEngine::getInstance()->preloadEffect("sound/enemy3_down.mp3");
        SimpleAudioEngine::getInstance()->preloadEffect("sound/get_double_laser.mp3");
        SimpleAudioEngine::getInstance()->preloadEffect("sound/use_bomb.mp3");
        SimpleAudioEngine::getInstance()->preloadEffect("sound/get_bomb.mp3");
        SimpleAudioEngine::getInstance()->preloadEffect("sound/big_spaceship_flying.mp3");

        SimpleAudioEngine::getInstance()->setBackgroundMusicVolume(0.5);
        SimpleAudioEngine::getInstance()->setEffectsVolume(1);

        bRet = true;
    } while(0);

    return bRet;
}

