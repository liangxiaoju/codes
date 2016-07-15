#include "WelcomeScene.h"
#include "MainMenuScene.h"
#include "BGLayer.h"
#include "Sound.h"

using namespace CocosDenshion;

bool WelcomeScene::init()
{
    if (!Scene::init())
        return false;

    auto layer = WelcomeLayer::create();
    addChild(layer);

    return true;
}

bool WelcomeLayer::init()
{
    if (!Layer::init())
        return false;

    Size vsize = Director::getInstance()->getVisibleSize();

    auto bg = BGLayer::create();
    addChild(bg);

    auto rotateBy = RotateBy::create(0.1, -15);
    Repeat *repeat = Repeat::create(rotateBy, 10);
    CallFunc *callback = CallFunc::create([]() {
            Director::getInstance()->replaceScene(MainMenuScene::create());
        });
    Sequence *seq = Sequence::create(repeat, callback, nullptr);

    auto sp1 = Sprite::create("head/load_outter.png");
    sp1->runAction(seq);
    sp1->setPosition(vsize.width/2, vsize.height/2);
    addChild(sp1, 0, "outter");

    auto sp2 = Sprite::create("load_inner.png");
    sp2->setPosition(vsize.width/2, vsize.height/2);
    addChild(sp2, 0, "inner");

    Sound::getInstance()->playBackgroundMusic();

    return true;
}
