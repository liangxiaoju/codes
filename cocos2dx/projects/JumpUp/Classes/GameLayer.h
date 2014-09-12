#ifndef __GAMELAYER_H__
#define __GAMELAYER_H__

#include "cocos2d.h"

USING_NS_CC;

class GameLayer : public Layer {
public:
    virtual bool init();
    CREATE_FUNC(GameLayer);

private:
    void onEnter();
    void onExit();
    bool onTouchBegan(Touch *touch, Event *event);
    void onTouchMoved(Touch *touch, Event *event);
    void onTouchEnded(Touch *touch, Event *event);
};

#endif
