#ifndef __UFOSPRITE_H__
#define __UFOSPRITE_H__

#include "cocos2d.h"

USING_NS_CC;

class UFOSprite : public Sprite {
public:
    bool initWithType(int type);
    static UFOSprite *create(int type);
    int getType();

private:
    enum {
        TYPE_UFO1 = 1,
        TYPE_UFO2,
        TYPE_UFO3,
    };

    int mType;
};

#endif
