#ifndef __HEADERVIEW_H__
#define __HEADERVIEW_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "Board.h"
#include "LevelMenu.h"

USING_NS_CC;
using namespace cocos2d::ui;

class EnemyHeaderView : public RelativeBox
{
public:
    typedef Board::Side Side;

    virtual bool init(int level, Side side);
    static EnemyHeaderView *create(int level, Side side)
    {
        auto pRet = new (std::nothrow) EnemyHeaderView();
        if (pRet && pRet->init(level, side)) {
            pRet->autorelease();
            return pRet;
        }
        CC_SAFE_DELETE(pRet);
        return nullptr;
    }

private:
    int _level;
    int _step;
    Side _side;

    ImageView *_head;
    Text *_text;
    ImageView *_piece;
};

class SelfHeaderView : public RelativeBox
{
public:
    virtual bool init();
    CREATE_FUNC(SelfHeaderView);
};

#endif
