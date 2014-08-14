#ifndef __ENEMYSPRITE_H__
#define __ENEMYSPRITE_H__

#include "cocos2d.h"
#include "ScoreLayer.h"

USING_NS_CC;

class EnemySprite : public Sprite {
public:
    bool initWithType(int type, float multiple);
    static EnemySprite *create(float multiple);
    static EnemySprite *create(int type, float multiple);
    void blowUp();
    void hit();
    bool isAlive();

private:

    enum {
        TYPE_ENEMY1 = 1,
        TYPE_ENEMY2,
        TYPE_ENEMY3,
    };

    int mType;
    int mLife;
    int mCost;
    int mBlowingUp;
};

#endif
