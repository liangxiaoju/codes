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
    MenuItemSprite *mBombSprite;
    MenuItemSprite *mPauseSprite;
    Label *mBombLabel;

    void update(float fDelta) override;

    void gameOver();
    void gamePause();

    void showEnemy(float dt);
    void showUFO(float dt);

    void useBomb();
    void updateBomb();
    void bombAllEnemys();

    void onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event);
};

#endif
