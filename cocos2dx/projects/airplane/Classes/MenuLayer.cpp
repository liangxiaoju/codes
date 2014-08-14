#include "MenuLayer.h"
#include "AdController.h"

using namespace CocosDenshion;

bool MenuLayer::init() {
    bool bRet = false;

    do {
        CC_BREAK_IF(!Menu::init());

        MenuItemFont::setFontSize(50);
        addChild(MenuItemFont::create("Return", CC_CALLBACK_1(MenuLayer::returnMenuCallback, this)));
        addChild(MenuItemFont::create("Restart", CC_CALLBACK_1(MenuLayer::restartMenuCallback, this)));
        addChild(MenuItemFont::create("Exit", CC_CALLBACK_1(MenuLayer::exitMenuCallback, this)));
        alignItemsVerticallyWithPadding(20);

        setKeyboardEnabled(true);
        setVisible(false);

        bRet = true;
    } while (0);

    return bRet;
}

void MenuLayer::show() {
    Director::getInstance()->pause();
    setVisible(true);
    AdController::getInstance()->setVisible(true);

    SimpleAudioEngine::getInstance()->pauseAllEffects();
    SimpleAudioEngine::getInstance()->pauseBackgroundMusic();
}

void MenuLayer::hide() {
    setVisible(false);
    Director::getInstance()->resume();
    AdController::getInstance()->setVisible(false);

    SimpleAudioEngine::getInstance()->resumeAllEffects();
    SimpleAudioEngine::getInstance()->resumeBackgroundMusic();
}

void MenuLayer::returnMenuCallback(Ref *sender) {
    hide();
}

void MenuLayer::restartMenuCallback(Ref *sender) {
    GameLayer::getInstance()->restart();
    hide();
}

void MenuLayer::exitMenuCallback(Ref *sender) {
    Director::getInstance()->end();
}

void MenuLayer::onKeyReleased(EventKeyboard::KeyCode keyCode, Event* unused) {
    switch (keyCode) {
    case EventKeyboard::KeyCode::KEY_BACK:
        if (isVisible()) {
            hide();
        } else {
            show();
        }
        break;
    default:
        break;
    }
}

