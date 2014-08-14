#ifndef __GAMESCENE_H__
#define __GAMESCENE_H__

#include "cocos2d.h"
#include "BackgroundLayer.h"
#include "GameLayer.h"
#include "ScoreLayer.h"
#include "MenuLayer.h"

class GameScene : public cocos2d::Scene
{
public:
    GameScene();
    virtual ~GameScene();

    virtual bool init();

    CREATE_FUNC(GameScene);

private:
    BackgroundLayer *mBackgroundLayer;
    GameLayer *mGameLayer;
    ScoreLayer *mScoreLayer;
    MenuLayer *mMenuLayer;

    void onExit() override;
};

#endif
