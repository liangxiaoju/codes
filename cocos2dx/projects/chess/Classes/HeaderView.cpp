#include "HeaderView.h"
#include "Utils.h"
#include "LevelMenu.h"

bool EnemyHeaderView::init(Side side)
{
    if (!RelativeBox::init())
        return false;

    RelativeLayoutParameter *param;

    int level = LevelMenu::getUserLevel();
    std::vector<std::string> vec = {
        "head_level1.png",
        "head_level1.png",
        "head_level2.png",
        "head_level2.png",
        "head_level2.png",
        "head_level3.png",
        "head_level3.png",
        "head_level4.png"
    };

    auto head = ImageView::create("common/"+vec[level]);
    param = RelativeLayoutParameter::create();
    param->setAlign(
        RelativeLayoutParameter::RelativeAlign::PARENT_LEFT_CENTER_VERTICAL);
    param->setRelativeName("head");
    head->setLayoutParameter(param);
    addChild(head);

    auto str = LevelMenu::getUserLevelString();

    auto text = Text::create(str, "", 60);
    text->setTextColor(Color4B(88, 49, 14, 255));
    param = RelativeLayoutParameter::create();
    param->setMargin(Margin(0, 60, 0, 0));
    param->setAlign(
        RelativeLayoutParameter::RelativeAlign::LOCATION_RIGHT_OF_TOPALIGN);
    param->setRelativeName("level");
    param->setRelativeToWidgetName("head");
    text->setLayoutParameter(param);
    addChild(text);

    std::string filename = side == Side::BLACK ? "BK.png" : "RK.png";
    auto piece = ImageView::create("common/"+filename);
    param = RelativeLayoutParameter::create();
    param->setAlign(
        RelativeLayoutParameter::RelativeAlign::LOCATION_BELOW_LEFTALIGN);
    param->setRelativeToWidgetName("level");
    piece->setLayoutParameter(param);
    addChild(piece);

    auto headSize = head->getContentSize();
    auto textSize = text->getContentSize();
    auto pieceSize = piece->getContentSize();
    float w = headSize.width+std::max(textSize.width, pieceSize.width);
    float h = headSize.height;
    setContentSize(Size(w, h));

    setAnchorPoint(Vec2(0.5, 0.5));

    _head = head;
    _text = text;
    _piece = piece;

    setActive(false);

    setOnEnterCallback([this, side, vec]() {
            getEventDispatcher()->addCustomEventListener(
                    EVENT_WHITE_START,
                    [this, side](EventCustom *ev) {
                    setActive(side==Side::WHITE);
                    });
            getEventDispatcher()->addCustomEventListener(
                    EVENT_BLACK_START,
                    [this, side](EventCustom *ev) {
                    setActive(side==Side::BLACK);
                    });
            getEventDispatcher()->addCustomEventListener(
                    EVENT_LEVEL_CHANGE,
                    [this, vec](EventCustom *ev) {
                    int level = LevelMenu::getUserLevel();
                    _head->loadTexture("common/"+vec[level]);
                    auto str = LevelMenu::getUserLevelString();
                    _text->setString(str);
                    requestDoLayout();
                    });
            });
    setOnExitCallback([this]() {
            getEventDispatcher()->removeCustomEventListeners(
                    EVENT_WHITE_START);
            getEventDispatcher()->removeCustomEventListeners(
                    EVENT_BLACK_START);
            getEventDispatcher()->removeCustomEventListeners(
                    EVENT_LEVEL_CHANGE);
            });

    return true;
}

void EnemyHeaderView::setActive(bool active)
{
    _piece->setVisible(active);
}

bool SelfHeaderView::init(Side side)
{
    if (!RelativeBox::init())
        return false;

    RelativeLayoutParameter *param;

    auto head = ImageView::create("common/head_level1.png");
    head->setFlippedX(true);
    param = RelativeLayoutParameter::create();
    param->setAlign(
        RelativeLayoutParameter::RelativeAlign::PARENT_RIGHT_CENTER_VERTICAL);
    param->setRelativeName("head");
    head->setLayoutParameter(param);
    addChild(head);

    auto time = Text::create(TR("time:")+"00:00", "", 32);
    time->setTextColor(Color4B(88, 49, 14, 255));
    param = RelativeLayoutParameter::create();
    param->setAlign(
        RelativeLayoutParameter::RelativeAlign::LOCATION_LEFT_OF_BOTTOMALIGN);
    param->setRelativeName("time");
    param->setRelativeToWidgetName("head");
    time->setLayoutParameter(param);
    addChild(time, 0, 0);

    auto updateTime = CallFunc::create([time]() {
            time->setTag(time->getTag()+1);
            int count = time->getTag();
            int min = count/60;
            int sec = count%60;
            char buf[16];
            snprintf(buf, sizeof(buf), "%02d:%02d", min, sec);
            time->setString(TR("time:")+buf);
            });
    auto action = RepeatForever::create(
            Sequence::create(DelayTime::create(1), updateTime, nullptr));
    time->runAction(action);

    auto step= Text::create(TR("step:")+"0", "", 60);
    step->setTextColor(Color4B(88, 49, 14, 255));
    param = RelativeLayoutParameter::create();
    param->setAlign(
        RelativeLayoutParameter::RelativeAlign::LOCATION_ABOVE_CENTER);
    param->setRelativeName("step");
    param->setRelativeToWidgetName("time");
    step->setLayoutParameter(param);
    addChild(step, 0, 0);

    std::string filename = side == Side::BLACK ? "BK.png" : "RK.png";
    auto piece = ImageView::create("common/"+filename);
    param = RelativeLayoutParameter::create();
    param->setAlign(
        RelativeLayoutParameter::RelativeAlign::LOCATION_ABOVE_CENTER);
    param->setRelativeToWidgetName("step");
    piece->setLayoutParameter(param);
    addChild(piece);

    auto headSize = head->getContentSize();
    auto stepSize = step->getContentSize();
    auto pieceSize = piece->getContentSize();
    float w = headSize.width+std::max(stepSize.width, pieceSize.width);
    float h = headSize.height;
    setContentSize(Size(w, h));

    setAnchorPoint(Vec2(0.5, 0.5));

    _head = head;
    _time = time;
    _step = step;
    _piece = piece;

    setActive(false);

    _stepCount = side==Side::WHITE ? 0 : -1;

    setOnEnterCallback([this, side]() {
            getEventDispatcher()->addCustomEventListener(
                    EVENT_WHITE_START,
                    [this, side](EventCustom *ev) {
                    setActive(side==Side::WHITE);
                    if (((++_stepCount) % 2) == 0)
                    this->step();
                    });
            getEventDispatcher()->addCustomEventListener(
                    EVENT_BLACK_START,
                    [this, side](EventCustom *ev) {
                    setActive(side==Side::BLACK);
                    if (((++_stepCount) % 2) == 0)
                    this->step();
                    });
            });
    setOnExitCallback([this]() {
            getEventDispatcher()->removeCustomEventListeners(
                    EVENT_WHITE_START);
            getEventDispatcher()->removeCustomEventListeners(
                    EVENT_BLACK_START);
            });

    return true;
}

void SelfHeaderView::setActive(bool active)
{
    _piece->setVisible(active);
}

void SelfHeaderView::step()
{
    _step->setTag(_step->getTag()+1);
    int count = _step->getTag();
    _step->setString(TR("step:")+Utils::toString(count));
}

