#ifndef __MENULAYER_H__
#define __MENULAYER_H__

#include "cocos2d.h"
#include "GameLayer.h"

USING_NS_CC;

class MenuLayer : public Menu {
public:
    virtual bool init();
    CREATE_FUNC(MenuLayer);

private:
    void returnMenuCallback(Ref *sender);
    void restartMenuCallback(Ref *sender);
    void exitMenuCallback(Ref *sender);
    void onKeyReleased(EventKeyboard::KeyCode keyCode, Event* unused);
    void show();
    void hide();
};

#endif
