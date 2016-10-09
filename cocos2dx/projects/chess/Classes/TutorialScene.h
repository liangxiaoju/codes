#ifndef __TUTORIALSCENE_H__
#define __TUTORIALSCENE_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "Board.h"
#include "UIPlayer.h"
#include "BGLayer.h"
#include "GameLayer.h"
#include "XQFile/XQFile.h"

USING_NS_CC;
using namespace cocos2d::ui;

class TutorialScene : public Scene
{
public:
    virtual bool init(XQFile *xqFile);
    static TutorialScene* create(XQFile *xqFile)
    {
        auto *pRet = new (std::nothrow) TutorialScene();
        if (pRet && pRet->init(xqFile)) {
            pRet->autorelease();
            return pRet;
        } else {
            delete pRet;
            pRet = nullptr;
            return nullptr;
        }
    }

    virtual ~TutorialScene();

private:
    XQFile *_xqFile;
    Board *_board;
    UIPlayer *_playerWhite;
    UIPlayer *_playerBlack;
};

class TutorialMenuScene: public Scene
{
public:
    bool init();
    CREATE_FUNC(TutorialMenuScene);
	void load(int pid) { _pid = pid; }

    ~TutorialMenuScene();

private:
    Button *_category_button;
    Button *_node_button;
    ListView *_listview;
    Layout *_headview;
    Layout *_layout;
	int _pid;
};

#endif
