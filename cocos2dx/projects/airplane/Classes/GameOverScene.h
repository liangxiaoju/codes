#ifndef __GAMEOVERSCENE_H__
#define __GAMEOVERSCENE_H__

#include "cocos2d.h"

USING_NS_CC;

class GameOverLayer : public Layer {
public:
    virtual bool init();
    CREATE_FUNC(GameOverLayer);

private:
    Label *mLabel;
    Menu *mMenu;

    void exitCallback(Ref *sender);
    void restartCallback(Ref *sender);
};

class GameOverScene : public Scene {
public:
    virtual bool init() { addChild(GameOverLayer::create()); return true; }

    CREATE_FUNC(GameOverScene);
};

#endif
