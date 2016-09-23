#ifndef __CONTROLLAYER_H__
#define __CONTROLLAYER_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "ControlPannel.h"

USING_NS_CC;
using namespace cocos2d::ui;

class ControlLayer : public Layer
{
public:
	virtual bool init();
	CREATE_FUNC(ControlLayer);

    virtual void initLayout();
    virtual std::vector<Widget*> getControlPannelWidgets();
    virtual std::vector<Widget*> generateControlPannelWidgets(
        std::vector<std::string>);
    virtual void initControlPannelButton();
    virtual void initControlPannel();
    virtual void initExitButton();
    virtual void initSettingMenu();
    virtual bool isControlPannelVisible();
    virtual void setControlPannelVisible(bool visible);
    virtual void setEnabled(bool enable);

    virtual ~ControlLayer();

protected:
    Layout *_layout;
    Button *_pannelButton;
    ControlPannel *_pannel;
    Button *_exitButton;
    Button *_settingButton;
};

class FightControlLayer : public ControlLayer
{
public:
    virtual bool init();
    CREATE_FUNC(FightControlLayer);
    virtual std::vector<Widget*> getControlPannelWidgets();
};

class ChallengeControlLayer : public ControlLayer
{
public:
    virtual bool init();
    CREATE_FUNC(ChallengeControlLayer);
    virtual std::vector<Widget*> getControlPannelWidgets();
};

class ResearchControlLayer : public ControlLayer
{
public:
    virtual bool init();
    CREATE_FUNC(ResearchControlLayer);
    virtual std::vector<Widget*> getControlPannelWidgets();
};

class LANFightControlLayer : public ControlLayer
{
public:
    virtual bool init();
    CREATE_FUNC(LANFightControlLayer);
    virtual std::vector<Widget*> getControlPannelWidgets();
};

class TutorialControlLayer : public ControlLayer
{
public:
    virtual bool init();
    CREATE_FUNC(TutorialControlLayer);
    virtual std::vector<Widget*> getControlPannelWidgets();
};

class LocalFightControlLayer : public ControlLayer
{
public:
    virtual bool init();
    CREATE_FUNC(LocalFightControlLayer);
    virtual std::vector<Widget*> getControlPannelWidgets();
    virtual void initControlPannelButton();
    virtual void initControlPannel();
    virtual void setControlPannelVisible(bool visible);
    void setMirrorControlPannelVisible(bool visible);
    virtual ~LocalFightControlLayer();

private:
    Button *_pannelButtonMirror;
    ControlPannel *_pannelMirror;
};

#endif
