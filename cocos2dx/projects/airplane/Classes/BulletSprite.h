#ifndef __BULLETSPRITE_H__
#define __BULLETSPRITE_H__

#include "cocos2d.h"

USING_NS_CC;

class BulletSprite : public Sprite {
public:
    bool initWithType(int type);
    static BulletSprite *create(int type);

    void start();

    void blowUp();
};

#endif
