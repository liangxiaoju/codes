#ifndef __BACKGROUNDLAYER_H__
#define __BACKGROUNDLAYER_H__

#include "cocos2d.h"

USING_NS_CC;

class BackgroundLayer: public Layer {
public:
    virtual bool init();

    CREATE_FUNC(BackgroundLayer);

    void startMove();
    void stopMove();
    void move(float dt);

private:
    Sprite *mBackground1;
    Sprite *mBackground2;
};


#endif
