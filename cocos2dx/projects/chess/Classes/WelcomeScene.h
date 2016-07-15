#ifndef __WELCOMESCENE_H__
#define __WELCOMESCENE_H__

#include "cocos2d.h"
#include "Utils.h"

USING_NS_CC;

class WelcomeLayer : public Layer {
public:
    virtual bool init();
    CREATE_FUNC(WelcomeLayer);
};

class WelcomeScene : public Scene {
public:
    virtual bool init();
    CREATE_FUNC(WelcomeScene);
};

#endif
