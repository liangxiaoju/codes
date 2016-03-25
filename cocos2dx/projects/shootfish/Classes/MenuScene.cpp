#include "MenuScene.h"
#include "JavaHelper.h"
#include "SimpleAudioEngine.h"

using namespace CocosDenshion;

bool MenuLayer::init(RenderTexture *texture) {
    bool bRet = false;

    do {
        CC_BREAK_IF(!Layer::init());

        Sprite *sprite;
        if (texture != NULL) {
            sprite = Sprite::createWithTexture(texture->getSprite()->getTexture());
        } else {
            sprite = Sprite::createWithSpriteFrameName("bg.png");
        }
        sprite->setFlippedY(true);
        sprite->setAnchorPoint(Vec2(0, 0));
        sprite->setPosition(0, 0);
        addChild(sprite);

        auto s = Director::getInstance()->getVisibleSize();
        SpriteFrame *sf = Sprite::create()->getSpriteFrame();

        Scale9Sprite *s1 = Scale9Sprite::createWithSpriteFrame(sf);
        s1->setColor(Color3B::GRAY);
        //auto l1 = Label::createWithTTF("Back", "fonts/Marker Felt.ttf", 30);
        auto l1 = Label::createWithSystemFont("Back", "arial", 70);
        l1->setColor(Color3B::BLACK);
        ControlButton *btn_back = ControlButton::create(l1, s1);
        //btn_back->setTitleColorForState(Color3B::WHITE, Control::State::HIGH_LIGHTED);
        btn_back->addTargetWithActionForControlEvents(this,
                cccontrol_selector(MenuLayer::returnMenuCallback), Control::EventType::TOUCH_UP_INSIDE);
        btn_back->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
        btn_back->setPosition(s.width/2, s.height/2+180);
        btn_back->setPreferredSize(Size(s.width*2/3, 100));

        Scale9Sprite *s2 = Scale9Sprite::createWithSpriteFrame(sf);
        s2->setColor(Color3B::GRAY);
        auto l2 = Label::createWithSystemFont("Restart", "arial", 70);
        l2->setColor(Color3B::BLACK);
        ControlButton *btn_restart = ControlButton::create(l2, s2);
        btn_restart->addTargetWithActionForControlEvents(this,
                cccontrol_selector(MenuLayer::restartMenuCallback), Control::EventType::TOUCH_UP_INSIDE);
        btn_restart->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
        btn_restart->setPosition(s.width/2, s.height/2);
        btn_restart->setPreferredSize(Size(s.width*2/3, 100));

        Scale9Sprite *s3 = Scale9Sprite::createWithSpriteFrame(sf);
        s3->setColor(Color3B::GRAY);
        auto l3 = Label::createWithSystemFont("Exit", "arial", 70);
        l3->setColor(Color3B::BLACK);
        ControlButton *btn_exit = ControlButton::create(l3, s3);
        btn_exit->addTargetWithActionForControlEvents(this,
                cccontrol_selector(MenuLayer::exitMenuCallback), Control::EventType::TOUCH_UP_INSIDE);
        btn_exit->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
        btn_exit->setPosition(s.width/2, s.height/2-180);
        btn_exit->setPreferredSize(Size(s.width*2/3, 100));

        addChild(btn_back);
        addChild(btn_restart);
        addChild(btn_exit);

        setKeyboardEnabled(true);

        show();

        bRet = true;
    } while (0);

    return bRet;
}

void MenuLayer::show() {
    JavaHelper::getInstance()->showAds();
    SimpleAudioEngine::getInstance()->pauseAllEffects();
    SimpleAudioEngine::getInstance()->pauseBackgroundMusic();
}

void MenuLayer::hide() {
    JavaHelper::getInstance()->hideAds();
    SimpleAudioEngine::getInstance()->resumeAllEffects();
    SimpleAudioEngine::getInstance()->resumeBackgroundMusic();
    Director::getInstance()->popScene();
}

void MenuLayer::returnMenuCallback(Ref *sender, Control::EventType controlEvent) {
    hide();
}

void MenuLayer::restartMenuCallback(Ref *sender, Control::EventType controlEvent) {
    GameLayer::getInstance()->restart();
    ScoreLayer::getInstance()->clear();
    hide();
}

void MenuLayer::exitMenuCallback(Ref *sender, Control::EventType controlEvent) {
    Director::getInstance()->end();
}

void MenuLayer::onKeyReleased(EventKeyboard::KeyCode keyCode, Event* unused) {
    switch (keyCode) {
    case EventKeyboard::KeyCode::KEY_BACK:
        hide();
        break;
    default:
        break;
    }
}

