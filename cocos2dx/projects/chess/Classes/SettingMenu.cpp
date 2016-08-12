#include "SettingMenu.h"
#include "UserData.h"
#include "Sound.h"

class CheckBoxItem : public Widget
{
public:
    typedef std::function<void(bool selected)> CheckBoxItemCallback;

    static CheckBoxItem* create(std::string content, CheckBoxItemCallback cb,
                                bool defaultVal = false) {
        auto widget = new (std::nothrow) CheckBoxItem();
        if (widget && widget->init(content, cb, defaultVal)) {
            widget->autorelease();
            return widget;
        }
        CC_SAFE_DELETE(widget);
        return nullptr;
    }
    virtual bool init(std::string content, CheckBoxItemCallback cb, bool defaultVal) {
        auto wsize = Director::getInstance()->getWinSize();

        auto layout = RelativeBox::create();
        addChild(layout);

        RelativeLayoutParameter *param;

        param = RelativeLayoutParameter::create();
        param->setAlign(
            RelativeLayoutParameter::RelativeAlign::PARENT_LEFT_CENTER_VERTICAL);
        auto text = Text::create(content, "fonts/arial.ttf", 35);
        text->setTextColor(Color4B::BLACK);
        text->setLayoutParameter(param);

        param = RelativeLayoutParameter::create();
        param->setAlign(
            RelativeLayoutParameter::RelativeAlign::PARENT_RIGHT_CENTER_VERTICAL);
        auto checkbox = CheckBox::create("check_box_normal.png",
                                         "check_box_active.png");
        checkbox->setSelected(defaultVal);
        checkbox->setScale(1.5);
        checkbox->setLayoutParameter(param);
        checkbox->addEventListener([cb](Ref *pSender, CheckBox::EventType type) {
                switch (type)
                {
                case CheckBox::EventType::SELECTED:
                    cb(true);
                    break;
                case CheckBox::EventType::UNSELECTED:
                    cb(false);
                    break;
                default:
                    break;
                }
            });

        layout->addChild(text);
        layout->addChild(checkbox);

        layout->setContentSize(Size(wsize.width/2,
                            std::max(text->getContentSize().height,
                                     checkbox->getContentSize().height)));
        setContentSize(layout->getContentSize());

        return true;
    }
};

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

    music_cb(_music_enabled);
    effect_cb(_effect_enabled);
    tips_cb(_tips_enabled);

    pushBackView(CheckBoxItem::create("Background Music:", music_cb, _music_enabled));
    pushBackView(CheckBoxItem::create("Effect Sound:", effect_cb, _effect_enabled));
    pushBackView(CheckBoxItem::create("Tips:", tips_cb, _tips_enabled));

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

void SettingMenu::load()
{
    _music_enabled = !!UserData::getInstance()->getIntegerForKey("MusicEnabled", 1);
    _effect_enabled = !!UserData::getInstance()->getIntegerForKey("EffectEnabled", 1);
    _tips_enabled = !!UserData::getInstance()->getIntegerForKey("TipsEnabled", 0);
}

void SettingMenu::save()
{
    UserData::getInstance()->setIntegerForKey("MusicEnabled", _music_enabled ? 1 : 0);
    UserData::getInstance()->setIntegerForKey("EffectEnabled", _effect_enabled ? 1 : 0);
    UserData::getInstance()->setIntegerForKey("TipsEnabled", _tips_enabled ? 1 : 0);
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
