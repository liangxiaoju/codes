#ifndef __REPLAYSCENE_H__
#define __REPLAYSCENE_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"

USING_NS_CC;
using namespace cocos2d::ui;

class ReplayScene : public Scene
{
public:
	virtual bool init();
	CREATE_FUNC(ReplayScene);

private:
	ListView *_listview;
	Layout *_default_model;
};

#endif
