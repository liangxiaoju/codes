#ifndef __SCHOOLSCENE_H__
#define __SCHOOLSCENE_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "Board.h"
#include "UIPlayer.h"
#include "BGLayer.h"
#include "GameLayer.h"
#include "XQFile/XQFile.h"

USING_NS_CC;
using namespace cocos2d::ui;

class SchoolScene : public Scene
{
public:
    virtual bool init(XQFile *xqFile);
    static SchoolScene* create(XQFile *xqFile)
    {
        SchoolScene *pRet = new (std::nothrow) SchoolScene();
        if (pRet && pRet->init(xqFile)) {
            pRet->autorelease();
            return pRet;
        } else {
            delete pRet;
            pRet = nullptr;
            return nullptr;
        }
    }

    virtual ~SchoolScene();

private:
    XQFile *_xqFile;
    Board *_board;
    UIPlayer *_playerWhite;
    UIPlayer *_playerBlack;
};

#endif
