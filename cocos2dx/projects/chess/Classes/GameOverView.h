#ifndef __GAMEOVERVIEW_H__
#define __GAMEOVERVIEW_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "PopupBox.h"

USING_NS_CC;
using namespace cocos2d::ui;

class GameOverView : public PopupBox
{
public:
    virtual bool init(std::string headfile,
                      std::string textstr,
                      std::string capturefile);
};

class FightWinView : public GameOverView
{
public:
    virtual bool init() override;
    CREATE_FUNC(FightWinView);
};

class FightLoseView : public GameOverView
{
public:
    virtual bool init() override;
    CREATE_FUNC(FightLoseView);
};

class FightDrawView : public GameOverView
{
public:
    virtual bool init() override;
    CREATE_FUNC(FightDrawView);
};

#endif
