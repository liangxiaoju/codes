#ifndef __MAINMENUSCENE_H__
#define __MAINMENUSCENE_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"

USING_NS_CC;
using namespace cocos2d::ui;

class MainMenuLayer : public Layer
{
public:
	virtual bool init();
	CREATE_FUNC(MainMenuLayer);
};

class MainMenuScene : public Scene
{
public:
	virtual bool init();
	CREATE_FUNC(MainMenuScene);
};

#endif
