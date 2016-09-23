#include "SettingMenu.h"
#include "UserData.h"
#include "Sound.h"
#include "Localization.h"
#include "CheckBoxItem.h"

SettingMenu *SettingMenu::s_settingMenu = nullptr;

SettingMenu *SettingMenu::getInstance() {
    if (!s_settingMenu) {
        s_settingMenu = new (std::nothrow) SettingMenu();
        if (s_settingMenu)
            s_settingMenu->init();
    }
    return s_settingMenu;
}

bool SettingMenu::init()
{
    if (!PopupBox::init())
        return false;

    load();

    auto music_cb = [this](bool selected) {
        _music_enabled = selected;

        if (selected)
            Sound::getInstance()->playBackgroundMusic();
        else
            Sound::getInstance()->stopBackgroundMusic();

        save();
    };
    auto effect_cb = [this](bool selected) {
        _effect_enabled = selected;
        Sound::getInstance()->setEffectEnabled(selected);
        save();
    };
    auto tips_cb = [this](bool selected) {
        _tips_enabled = selected;
        save();
        log("tips enabled: %s", selected ? "true" : "false");
    };
    auto vibrate_cb = [this](bool selected) {
        _vibrate_enabled = selected;
        save();
    };

    music_cb(_music_enabled);
    effect_cb(_effect_enabled);
    tips_cb(_tips_enabled);
    vibrate_cb(_vibrate_enabled);

    pushBackView(CheckBoxItem::create(TR("Background Music:"), music_cb, _music_enabled));
    pushBackView(CheckBoxItem::create(TR("Game Effect Sound:"), effect_cb, _effect_enabled));
    pushBackView(CheckBoxItem::create(TR("Show Tips:"), tips_cb, _tips_enabled));
    pushBackView(CheckBoxItem::create(TR("Vibrate Effect:"), vibrate_cb, _vibrate_enabled));

    return true;
}

bool SettingMenu::onTouchBegan(Touch *touch, Event *event)
{
    Vec2 touchPos = touch->getLocation();
    Rect boundRect = getInnerBoundingBox();

    if (!boundRect.containsPoint(touchPos)) {
        hide();
    }

    return true;
}

bool SettingMenu::isMusicEnabled()
{
    return _music_enabled;
}

bool SettingMenu::isEffectEnabled()
{
    return _effect_enabled;
}

bool SettingMenu::isTipsEnabled()
{
    return _tips_enabled;
}

bool SettingMenu::isVibrateEnabled()
{
    return _vibrate_enabled;
}

void SettingMenu::load()
{
    _music_enabled = !!UserData::getInstance()->getIntegerForKey("MusicEnabled", 1);
    _effect_enabled = !!UserData::getInstance()->getIntegerForKey("EffectEnabled", 1);
    _tips_enabled = !!UserData::getInstance()->getIntegerForKey("TipsEnabled", 0);
    _vibrate_enabled = !!UserData::getInstance()->getIntegerForKey("VibrateEnabled", 1);
}

void SettingMenu::save()
{
    UserData::getInstance()->setIntegerForKey("MusicEnabled", _music_enabled ? 1 : 0);
    UserData::getInstance()->setIntegerForKey("EffectEnabled", _effect_enabled ? 1 : 0);
    UserData::getInstance()->setIntegerForKey("TipsEnabled", _tips_enabled ? 1 : 0);
    UserData::getInstance()->setIntegerForKey("VibrateEnabled", _vibrate_enabled ? 1 : 0);
}

void SettingMenu::show()
{
    if (getParent() == nullptr)
        Director::getInstance()->getRunningScene()->addChild(this);
}

void SettingMenu::hide()
{
    if (getParent())
        removeFromParent();
}
