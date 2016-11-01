#ifndef __HEADERVIEW_H__
#define __HEADERVIEW_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "Board.h"

USING_NS_CC;
using namespace cocos2d::ui;

class EnemyHeaderView : public RelativeBox
{
public:
    typedef Board::Side Side;

    virtual bool init(Side side);
    static EnemyHeaderView *create(Side side)
    {
        auto pRet = new (std::nothrow) EnemyHeaderView();
        if (pRet && pRet->init(side)) {
            pRet->autorelease();
            return pRet;
        }
        CC_SAFE_DELETE(pRet);
        return nullptr;
    }

    void setActive(bool active);

private:
    ImageView *_head;
    Text *_text;
    ImageView *_piece;
};

class SelfHeaderView : public RelativeBox
{
public:
    typedef Board::Side Side;

    virtual bool init(Side side);
    static SelfHeaderView *create(Side side)
    {
        auto pRet = new (std::nothrow) SelfHeaderView();
        if (pRet && pRet->init(side)) {
            pRet->autorelease();
            return pRet;
        }
        CC_SAFE_DELETE(pRet);
        return nullptr;
    }

    void setActive(bool active);
    void step();

private:
    ImageView *_head;
    Text *_time;
    Text *_step;
    ImageView *_piece;
    int _stepCount;
};

#endif
