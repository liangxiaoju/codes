#ifndef __LEVELMENU_H__
#define __LEVELMENU_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "PopupBox.h"

USING_NS_CC;
using namespace cocos2d::ui;

class LevelMenu : public PopupBox
{
public:
    virtual bool init() override;
    CREATE_FUNC(LevelMenu);
    bool onTouchBegan(Touch *touch, Event *event) override;
    static int getUserLevel();
    static void setUserLevel(int level);
    static std::string getUserLevelString();

private:
    void load();
    void save();
    int _userLevel;
};

#endif
