#ifndef __CORDSPRITE_H__
#define __CORDSPRITE_H__

#include "cocos2d.h"

USING_NS_CC;

class CordSprite : public Sprite {
public:
    virtual bool initWithVertex(Point a, Point b);
    static CordSprite *createWithVertex(Point a, Point b);
};

#endif
