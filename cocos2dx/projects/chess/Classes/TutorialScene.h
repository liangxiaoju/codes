#ifndef __TUTORIALSCENE_H__
#define __TUTORIALSCENE_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"

USING_NS_CC;
using namespace cocos2d::ui;

class TutorialScene: public Scene
{
public:
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
