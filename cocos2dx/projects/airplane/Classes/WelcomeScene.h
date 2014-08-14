#ifndef __WELCOMESCENE_H__
#define __WELCOMESCENE_H__

#include "cocos2d.h"
#include "GameScene.h"
#include "ScoreLayer.h"

USING_NS_CC;

class WelcomeLayer : public Layer {
public:
    virtual bool init();
    CREATE_FUNC(WelcomeLayer);

private:
    Sprite *mBg;
};

class WelcomeScene : public Scene {
public:
    virtual bool init();
    CREATE_FUNC(WelcomeScene);

private:
    WelcomeLayer *mLayer;
};

#endif
