#ifndef __MENUSCENE_H__
#define __MENUSCENE_H__

#include "cocos2d.h"
#include "GameLayer.h"
#include "ui/CocosGUI.h"
#include "extensions/cocos-ext.h"

USING_NS_CC;
USING_NS_CC_EXT;
using namespace ui;

class MenuLayer : public Layer {
public:
    virtual bool init(RenderTexture *texture);

    static MenuLayer *create(RenderTexture *texture) {
        MenuLayer *pRet = new MenuLayer();
        if (pRet && pRet->init(texture)) {
            pRet->autorelease();
            return pRet;
        } else {
            delete pRet;
            pRet = NULL;
            return NULL;
        }
    }

private:
    void returnMenuCallback(Ref *sender, Control::EventType controlEvent);
    void restartMenuCallback(Ref *sender, Control::EventType controlEvent);
    void exitMenuCallback(Ref *sender, Control::EventType controlEvent);
    void onKeyReleased(EventKeyboard::KeyCode keyCode, Event* unused);
    void show();
    void hide();
};

class MenuScene : public Scene {
public:
    virtual bool init(RenderTexture *texture) {
        auto layer = MenuLayer::create(texture);
        if (layer == NULL) return false;
        addChild(layer);
        return true;
    }

    static MenuScene *create(RenderTexture *texture) {
        MenuScene *pRet = new MenuScene();
        if (pRet && pRet->init(texture)) {
            pRet->autorelease();
            return pRet;
        } else {
            delete pRet;
            pRet = NULL;
            return NULL;
        }
    }
};

#endif
