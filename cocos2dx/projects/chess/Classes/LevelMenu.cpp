#include "LevelMenu.h"
#include "UserData.h"
#include "Localization.h"
#include "RadioButtonItem.h"
#include "Utils.h"

bool LevelMenu::init()
{
    if (!PopupBox::init())
        return false;

    _layout->setBackGroundImage("common/B2popupbox9_66x66_294x194@3x.png");
    _layout->setBackGroundImageCapInsets(Rect(66, 66, 294-66, 194-66));

    load();

    LinearLayoutParameter *param;

    auto head = Button::create("common/button3.png");
    head->setZoomScale(0.0f);
    head->setTitleText(TR("level"));
    head->setTitleFontSize(60);
    head->setTitleColor(Color3B(240, 213, 172));
    param = LinearLayoutParameter::create();
    param->setGravity(LinearLayoutParameter::LinearGravity::CENTER_HORIZONTAL);
    param->setMargin(Margin(0, 100, 0, 100));
    head->setLayoutParameter(param);
    addElement(head);

    auto text = Text::create(TR("LevelMenuString"), "", 40);
    text->setTextColor(Color4B(142, 86, 36, 255));
    param = LinearLayoutParameter::create();
    param->setGravity(LinearLayoutParameter::LinearGravity::CENTER_HORIZONTAL);
    text->setLayoutParameter(param);
    addElement(text);

    param = LinearLayoutParameter::create();
    param->setMargin(Margin(80, 0, 80, 0));

    auto line = ImageView::create("common/B3line@3x.png");
    line->setLayoutParameter(param->clone());
    addElement(line);

    auto listview = ListView::create();
    listview->setDirection(ListView::Direction::VERTICAL);
    listview->setBounceEnabled(true);
    listview->setScrollBarEnabled(false);
    listview->setGravity(ListView::Gravity::CENTER_HORIZONTAL);
    listview->addEventListener([this](Ref *pSender, ListView::EventType type) {
            ListView *listview = static_cast<ListView*>(pSender);
            switch (type) {
            case ListView::EventType::ON_SELECTED_ITEM_START:
                break;
            case ListView::EventType::ON_SELECTED_ITEM_END: {
                int index = listview->getCurSelectedIndex();
                _userLevel = index;
                save();

                getScheduler()->performFunctionInCocosThread([this, index]() {
                        Director::getInstance()->getEventDispatcher()
                            ->dispatchCustomEvent(EVENT_LEVEL_CHANGE, (void*)&index);
                        this->removeFromParent();
                    });
            }
            }
        });
    auto group = RadioButtonGroup::create();
    group->setAllowedNoSelection(false);
    addChild(group);

    std::vector<std::string> vec = {
        TR("level_0"),
        TR("level_1"),
        TR("level_2"),
        TR("level_3"),
        TR("level_4"),
        TR("level_5"),
        TR("level_6"),
        TR("level_7")
    };

    for (auto &name : vec) {
        auto button = RadioButtonItem::create(name);
        listview->pushBackCustomItem(button);
        group->addRadioButton(button);
    }

    group->setSelectedButtonWithoutEvent(_userLevel);

    auto size = listview->getItems().at(0)->getContentSize();
    listview->setContentSize(Size(size.width, size.height*6));
    param->setMargin(Margin(80, 0, 80, 40));
    listview->setLayoutParameter(param);
    addElement(listview);

    Director::getInstance()->getRunningScene()->addChild(this);

    return true;
}

void LevelMenu::load()
{
    _userLevel = UserData::getInstance()->getIntegerForKey("UserLevel", 0);
}

void LevelMenu::save()
{
    UserData::getInstance()->setIntegerForKey("UserLevel", _userLevel);
}

bool LevelMenu::onTouchBegan(Touch *touch, Event *event)
{
    Vec2 touchPos = touch->getLocation();
    Rect boundRect = getInnerBoundingBox();

    if (!boundRect.containsPoint(touchPos)) {
        removeFromParent();
    }

    return true;
}

int LevelMenu::getUserLevel()
{
    return UserData::getInstance()->getIntegerForKey("UserLevel", 0);
}

void LevelMenu::setUserLevel(int level)
{
    if (level < 0)
        level = 0;
    UserData::getInstance()->setIntegerForKey("UserLevel", level);
}

std::string LevelMenu::getUserLevelString()
{
    int level = LevelMenu::getUserLevel();

    std::vector<std::string> vec = {
        TR("level_0"),
        TR("level_1"),
        TR("level_2"),
        TR("level_3"),
        TR("level_4"),
        TR("level_5"),
        TR("level_6"),
        TR("level_7")
    };

    if (level >= vec.size())
        return "";

    return vec[level];
}
