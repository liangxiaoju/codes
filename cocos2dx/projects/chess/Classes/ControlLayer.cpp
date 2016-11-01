#include "ControlLayer.h"
#include "Utils.h"
#include "PopupBox.h"
#include "NumberButton.h"
#include "Localization.h"
#include "SettingMenu.h"

bool ControlLayer::init()
{
	if (!Layer::init())
		return false;

    initLayout();

	return true;
}

void ControlLayer::initLayout()
{
    auto vsize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

    _layout = RelativeBox::create(vsize);
    addChild(_layout);

    initControlPannelButton();
    initControlPannel();
    initExitButton();
    initSettingMenu();

    _pannel->retain();
}

ControlLayer::~ControlLayer()
{
    _pannel->release();
}

std::vector<Widget*> ControlLayer::getControlPannelWidgets()
{
    std::vector<std::string> names = {"save", "lose", "peace",
                                      "again", "regret", "switch"};
    return generateControlPannelWidgets(names);
}

std::vector<Widget*> ControlLayer::generateControlPannelWidgets(
    std::vector<std::string> names)
{
    std::vector<Widget*> widgets;
    std::map<std::string, std::pair<std::string, int>> maps = {
        {"exit", std::make_pair(EVENT_BACK, -1)},
        {"save", std::make_pair(EVENT_SAVE, -1)},
        {"lose", std::make_pair(EVENT_RESIGN, -1)},
        {"peace", std::make_pair(EVENT_DRAW, -1)},
        {"again", std::make_pair(EVENT_RESET, -1)},
        {"regret", std::make_pair(EVENT_REGRET, 3)},
        {"switch", std::make_pair(EVENT_SWITCH, -1)},
        {"tips", std::make_pair(EVENT_TIP, -1)},
        {"setting", std::make_pair(EVENT_SETTING, -1)},
    };

    std::string baseDir = "FightSceneMenu/";
    for (auto &name : names) {

        auto it = maps.find(name);
        if (it == maps.end())
            continue;

        std::string event = maps[name].first;
        int limit = maps[name].second;

        std::string normal = baseDir + "button_" + name + "_normal.png";
        std::string pressed = baseDir + "button_" + name + "_pressed.png";
        std::string disabled = baseDir + "button_" + name + "_disabled.png";
        if (!FileUtils::getInstance()->isFileExist(disabled))
            disabled = "";

        Button *button;
        if (limit > 0)
            button = NumberButton::create(normal, pressed, disabled, limit);
        else
            button = Button::create(normal, pressed, disabled);

        button->setName(name);

        button->addClickEventListener([this, event](Ref *ref) {
                setControlPannelVisible(false);
                getEventDispatcher()->dispatchCustomEvent(event);
            });

        widgets.push_back(button);
    }

    return widgets;
}

void ControlLayer::initControlPannelButton()
{
    _pannelButton = Button::create(
        "common/menu.png");
    _pannelButton->addClickEventListener([this](Ref *ref) {
            setControlPannelVisible(true);
        });
    _pannelButton->setTitleText(TR("Menu"));
    _pannelButton->setTitleFontSize(48);
    _pannelButton->setTitleColor(Color3B(231, 204, 143));

    auto param = RelativeLayoutParameter::create();
    param->setRelativeName("PannelButton");
    param->setAlign(RelativeLayoutParameter::RelativeAlign::PARENT_LEFT_BOTTOM);
    param->setMargin(Margin(0, 0, 0, 28));
    _pannelButton->setLayoutParameter(param);
    _layout->addChild(_pannelButton);
}

void ControlLayer::initControlPannel()
{
    auto param = RelativeLayoutParameter::create();
    param->setRelativeToWidgetName("PannelButton");
    param->setAlign(RelativeLayoutParameter::RelativeAlign::LOCATION_ABOVE_LEFTALIGN);
    _pannel = ControlPannel::create();
    _pannel->setLayoutParameter(param);
    for (auto &widget : getControlPannelWidgets())
        _pannel->addWidget(widget);
}

void ControlLayer::setControlPannelVisible(bool visible)
{
    if (visible) {
        if (_layout->getChildByName("pannel"))
            return;

        _layout->addChild(_pannel, 0, "pannel");

        auto listener = EventListenerTouchOneByOne::create();
        listener->onTouchBegan = [this](Touch *t, Event *e) {
            if (!isControlPannelVisible())
                return false;

            setControlPannelVisible(false);
            return true;
        };
        listener->setSwallowTouches(true);
        getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, _pannel);

    } else {
        if (!_layout->getChildByName("pannel"))
            return;

        getEventDispatcher()->removeEventListenersForTarget(_pannel);
        _layout->removeChildByName("pannel");
    }
}

bool ControlLayer::isControlPannelVisible()
{
    return !!_layout->getChildByName("pannel");
}

void ControlLayer::initExitButton()
{
    _exitButton = Button::create("FightSceneMenu/look_back.png");
    _exitButton->setZoomScale(0.1);
    _exitButton->addClickEventListener([](Ref *ref) {
            DialogBox::create(
                TR("Are you sure to exit ?"), TR("OK"), TR("Cancel"), [](bool yes) {
                    if (yes)
                        Director::getInstance()->popScene();
                }
                );
        });
    auto param = RelativeLayoutParameter::create();
    param->setAlign(RelativeLayoutParameter::RelativeAlign::PARENT_TOP_LEFT);
    param->setMargin(Margin(36, 41, 0, 0));
    _exitButton->setLayoutParameter(param);
    _layout->addChild(_exitButton);
}

void ControlLayer::initSettingMenu()
{
}

void ControlLayer::setEnabled(bool enable)
{
    _pannel->setEnabled(enable);
}

bool FightControlLayer::init()
{
    if (!ControlLayer::init())
        return false;

    auto over_callback = [this](EventCustom* ev) {

        setEnabled(true);

        std::vector<std::string> names = {"regret", "lose", "peace"};

        for (auto &name : names) {
            auto button = dynamic_cast<Button*>(_pannel->getChildByName(name));
            if (button) {
                button->setEnabled(false);
            }
        }

        if (SettingMenu::getInstance()->isVibrateEnabled())
            Device::vibrate(0.4);
    };

    setOnEnterCallback([this, over_callback](){
			getEventDispatcher()->addCustomEventListener(EVENT_GAMEOVER, over_callback);
			Device::setKeepScreenOn(true);
		});
    setOnExitCallback([this](){
			getEventDispatcher()->removeCustomEventListeners(EVENT_GAMEOVER);
			Device::setKeepScreenOn(false);
		});

    return true;
}

std::vector<Widget*> FightControlLayer::getControlPannelWidgets()
{
    std::vector<std::string> names = {"save", "lose", "peace",
                                      "again", "regret", "switch"};
    return generateControlPannelWidgets(names);
}

bool ChallengeControlLayer::init()
{
     if (!ControlLayer::init())
        return false;

    auto over_callback = [this](EventCustom* ev) {

        setEnabled(true);

        std::vector<std::string> names = {"regret", "lose"};

        for (auto &name : names) {
            auto button = dynamic_cast<Button*>(_pannel->getChildByName(name));
            if (button) {
                button->setEnabled(false);
            }
        }

        if (SettingMenu::getInstance()->isVibrateEnabled())
            Device::vibrate(0.4);
    };

    setOnEnterCallback([this, over_callback](){
			getEventDispatcher()->addCustomEventListener(EVENT_GAMEOVER, over_callback);
			Device::setKeepScreenOn(true);
		});
    setOnExitCallback([this](){
			getEventDispatcher()->removeCustomEventListeners(EVENT_GAMEOVER);
			Device::setKeepScreenOn(false);
		});

    return true;
}

std::vector<Widget*> ChallengeControlLayer::getControlPannelWidgets()
{
    std::vector<std::string> names = {"lose", "again", "regret"};
    return generateControlPannelWidgets(names);
}

bool ResearchControlLayer::init()
{
    if (!ControlLayer::init())
        return false;

    auto over_callback = [this](EventCustom* ev) {

        setEnabled(true);

        std::vector<std::string> names = {"regret"};

        for (auto &name : names) {
            auto button = dynamic_cast<Button*>(_pannel->getChildByName(name));
            if (button) {
                button->setEnabled(false);
            }
        }

        if (SettingMenu::getInstance()->isVibrateEnabled())
            Device::vibrate(0.4);
    };

    setOnEnterCallback([this, over_callback](){
			getEventDispatcher()->addCustomEventListener(EVENT_GAMEOVER, over_callback);
			Device::setKeepScreenOn(true);
		});
    setOnExitCallback([this](){
			getEventDispatcher()->removeCustomEventListeners(EVENT_GAMEOVER);
			Device::setKeepScreenOn(false);
		});

    return true;
}

std::vector<Widget*> ResearchControlLayer::getControlPannelWidgets()
{
    std::vector<std::string> names = {"save", "again", "regret", "switch"};
    return generateControlPannelWidgets(names);
}

bool LANFightControlLayer::init()
{
    if (!ControlLayer::init())
        return false;

    auto over_callback = [this](EventCustom* ev) {

        setEnabled(true);

        std::vector<std::string> names = {"regret", "lose", "peace"};

        for (auto &name : names) {
            auto button = dynamic_cast<Button*>(_pannel->getChildByName(name));
            if (button) {
                button->setEnabled(false);
            }
        }

        if (SettingMenu::getInstance()->isVibrateEnabled())
            Device::vibrate(0.4);
    };

    setOnEnterCallback([this, over_callback](){
			getEventDispatcher()->addCustomEventListener(EVENT_GAMEOVER, over_callback);
			Device::setKeepScreenOn(true);
		});
    setOnExitCallback([this](){
			getEventDispatcher()->removeCustomEventListeners(EVENT_GAMEOVER);
			Device::setKeepScreenOn(false);
		});

    return true;
}

std::vector<Widget*> LANFightControlLayer::getControlPannelWidgets()
{
    std::vector<std::string> names = {"save", "lose", "peace",
                                      "again", "regret"};
    return generateControlPannelWidgets(names);
}

bool TutorialControlLayer::init()
{
    if (!ControlLayer::init())
        return false;

    setOnEnterCallback([this](){
			Device::setKeepScreenOn(true);
		});
    setOnExitCallback([this](){
			Device::setKeepScreenOn(false);
		});

    return true;
}

std::vector<Widget*> TutorialControlLayer::getControlPannelWidgets()
{
    std::vector<std::string> names = {"save", "again"};
    return generateControlPannelWidgets(names);
}

bool LocalFightControlLayer::init()
{
    if (!ControlLayer::init())
        return false;

    setOnEnterCallback([this](){
			Device::setKeepScreenOn(true);
		});
    setOnExitCallback([this](){
			Device::setKeepScreenOn(false);
		});

    return true;
}

std::vector<Widget*> LocalFightControlLayer::getControlPannelWidgets()
{
    std::vector<std::string> names = {"save", "again"};
    return generateControlPannelWidgets(names);
}

void LocalFightControlLayer::initControlPannelButton()
{
    ControlLayer::initControlPannelButton();

    _pannelButtonMirror = Button::create(
        "FightSceneMenu/common_push.png",
        "FightSceneMenu/common_pushP.png",
        "FightSceneMenu/common_push_dis.png");
    _pannelButtonMirror->setRotation(180);
    _pannelButtonMirror->addClickEventListener([this](Ref *ref) {
            setMirrorControlPannelVisible(true);
        });
    auto param = RelativeLayoutParameter::create();
    param->setRelativeName("PannelButtonMirror");
    param->setAlign(RelativeLayoutParameter::RelativeAlign::PARENT_TOP_RIGHT);
    param->setMargin(Margin(0, 40, 35, 0));
    _pannelButtonMirror->setLayoutParameter(param);
    _layout->addChild(_pannelButtonMirror);
}

void LocalFightControlLayer::initControlPannel()
{
    ControlLayer::initControlPannel();

    auto param = RelativeLayoutParameter::create();
    param->setRelativeToWidgetName("PannelButtonMirror");
    param->setAlign(RelativeLayoutParameter::RelativeAlign::LOCATION_BELOW_RIGHTALIGN);
    _pannelMirror = ControlPannel::create();
    _pannelMirror->setAnchorPoint(Vec2(0.5, 0.5));
    _pannelMirror->setRotation(180);
    _pannelMirror->setLayoutParameter(param);
    for (auto &widget : getControlPannelWidgets())
        _pannelMirror->addWidget(widget);

    _pannelMirror->retain();
}

LocalFightControlLayer::~LocalFightControlLayer()
{
    _pannelMirror->release();
}

void LocalFightControlLayer::setMirrorControlPannelVisible(bool visible)
{
    if (visible) {
        if (_layout->getChildByName("pannelMirror"))
            return;

        _layout->addChild(_pannelMirror, 0, "pannelMirror");

        auto listener = EventListenerTouchOneByOne::create();
        listener->onTouchBegan = [this](Touch *t, Event *e) {
            if (!_layout->getChildByName("pannelMirror"))
                return false;

            setMirrorControlPannelVisible(false);
            return true;
        };
        listener->setSwallowTouches(true);
        getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, _pannelMirror);
    } else {
        if (!_layout->getChildByName("pannelMirror"))
            return;
        getEventDispatcher()->removeEventListenersForTarget(_pannelMirror);
        _layout->removeChildByName("pannelMirror");
    }
}

void LocalFightControlLayer::setControlPannelVisible(bool visible)
{
    ControlLayer::setControlPannelVisible(visible);

    if (!visible) {
        setMirrorControlPannelVisible(false);
    }
}

