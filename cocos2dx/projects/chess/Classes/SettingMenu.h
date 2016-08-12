#ifndef __SETTINGMENU_H__
#define __SETTINGMENU_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "PopupBox.h"

USING_NS_CC;
using namespace cocos2d::ui;

class SettingMenu : public PopupBox
{
public:
    static SettingMenu *getInstance();
    virtual bool init() override;
    bool isMusicEnabled();
    bool isEffectEnabled();
    bool isTipsEnabled();
    void load();
    void save();
    void show();
    void hide();

private:
    bool onTouchBegan(Touch *touch, Event *event) override;
    static SettingMenu *s_settingMenu;
    bool _music_enabled;
    bool _effect_enabled;
    bool _tips_enabled;
};

#endif
