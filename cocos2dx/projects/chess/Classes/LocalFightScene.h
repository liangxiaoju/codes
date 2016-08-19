#ifndef __LOCALFIGHTSCENE_H__
#define __LOCALFIGHTSCENE_H__

#include "cocos2d.h"
#include "Board.h"
#include "BGLayer.h"
#include "GameLayer.h"
#include "UIPlayer.h"
#include "UserData.h"
#include "ControlLayer.h"

USING_NS_CC;

class LocalFightScene : public Scene
{
public:
    virtual bool init();
    CREATE_FUNC(LocalFightScene);

private:
    Board *_board;
    UIPlayer *_playerWhite;
    UIPlayer *_playerBlack;
    GameLayer *_gameLayer;
};

#endif
