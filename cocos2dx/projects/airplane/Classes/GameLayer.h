#ifndef __GAMELAYER_H__
#define __GAMELAYER_H__

#include "cocos2d.h"
#include "PlaneSprite.h"
#include "EnemySprite.h"
#include "GameOverScene.h"

USING_NS_CC;

class PlaneSprite;

class GameLayer : public Layer {
public:
    virtual bool init();
    static GameLayer *getInstance();
    void restart();

private:
    PlaneSprite *mPlaneSprite;
    static GameLayer *s_gameLayer;

    int mBomb;
    Sprite *mBombSprite;
    Label *mBombLabel;

    void repeat(float dt);

    void update(float fDelta) override;

    void gameover();

    void showUFO(float dt);

    void updateBomb();
    void bombAllEnemys();
};

#endif
