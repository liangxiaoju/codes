#include "GameOverView.h"
#include "Localization.h"
#include "UserData.h"
#include "Sound.h"
#include "Utils.h"
#include "LevelMenu.h"

bool GameOverView::init(
    std::string headfile, std::string textstr, std::string capturefile)
{
    if (!PopupBox::init())
        return false;

    _layout->removeBackGroundImage();

    LinearLayoutParameter *param;
    param = LinearLayoutParameter::create();
    param->setGravity(LinearLayoutParameter::LinearGravity::CENTER_HORIZONTAL);

    auto head = ImageView::create(headfile);
    head->setLayoutParameter(param->clone());
    addElement(head);

    auto text = Text::create(textstr, "", 60);
    text->setTextColor(Color4B(240, 213, 172, 255));
    text->setAnchorPoint(Vec2(0.5, 0));
    text->setPosition(Vec2(head->getContentSize().width/2, 30));
    head->addChild(text);

    auto layout = Layout::create();
    layout->setLayoutParameter(param->clone());
    auto bg = Sprite::create("common/background.png", Rect(0, 340, 1080, 1240));
    bg->setScale(0.713f);
    auto size = bg->getContentSize() * bg->getScale();
    bg->setPosition(size.width/2, size.height/2);
    layout->setContentSize(size);
    layout->addChild(bg);
    auto capture = ImageView::create(capturefile);
    if (capture) {
        capture->setAnchorPoint(Vec2(0.5, 0.5));
        capture->setPosition(Vec2(size.width/2, size.height/2));
        layout->addChild(capture);
        addElement(layout);
    }

    auto b1 = Button::create("common/button1.png");
    b1->setTitleText(TR("continue game"));
    b1->setTitleFontSize(60);
    b1->setTitleColor(Color3B(240, 213, 172));
    b1->addClickEventListener([this](Ref *ref) {
            //removeFromParent();
            getScheduler()->performFunctionInCocosThread([this]() {
                    getEventDispatcher()->dispatchCustomEvent(EVENT_RESET);
                });
        });

    auto b2 = Button::create("common/button2.png");
    b2->setTitleText(TR("back to home"));
    b2->setTitleFontSize(60);
    b2->setTitleColor(Color3B(240, 213, 172));
    b2->addClickEventListener([](Ref *ref) {
            Director::getInstance()->popScene();
            });

    auto bsize = b1->getContentSize();
    auto hbox = HBox::create(Size(bsize.width*2+84, bsize.height));
    param = LinearLayoutParameter::create();
    param->setGravity(LinearLayoutParameter::LinearGravity::CENTER_HORIZONTAL);
    param->setMargin(Margin(0.0f, 92.0f, 0.0f, 0.0f));
    hbox->setLayoutParameter(param);

    param = LinearLayoutParameter::create();
    param->setGravity(LinearLayoutParameter::LinearGravity::LEFT);
    param->setMargin(Margin(0.0f, 0.0f, 0.0f, 0.0f));
    b2->setLayoutParameter(param->clone());

    param->setGravity(LinearLayoutParameter::LinearGravity::RIGHT);
    param->setMargin(Margin(84, 0.0f, 0.0f, 0.0f));
    b1->setLayoutParameter(param->clone());

    hbox->addChild(b2);
    hbox->addChild(b1);
    addElement(hbox);

    Director::getInstance()->getRunningScene()->addChild(this);

    return true;
}

bool FightWinView::init()
{
    Sound::getInstance()->playEffect("win");
    int count = UserData::getInstance()->getIntegerForKey("FightScene:WIN", 0);
    UserData::getInstance()->setIntegerForKey("FightScene:WIN", count+1);

    int win_cont = UserData::getInstance()->getIntegerForKey("FightScene:WIN_CONT", 0);
    UserData::getInstance()->setIntegerForKey("FightScene:WIN_CONT", win_cont+1);
    UserData::getInstance()->setIntegerForKey("FightScene:LOSE_CONT", 0);

    std::string text;
    if (win_cont+1 >= 3) {
        UserData::getInstance()->setIntegerForKey("FightScene:WIN_CONT", 0);
        int level = LevelMenu::getUserLevel();
        if (level >= 7) {
            text = TR("FightWinString");
        } else {
            LevelMenu::setUserLevel(level+1);
            auto levelStr = LevelMenu::getUserLevelString();

            text = TR("FightContWinString");
            if (text.find("%s") != std::string ::npos) {
                char buf[256];
                snprintf(buf, sizeof(buf), text.c_str(), levelStr.c_str());
                text = buf;
            }
        }
    } else
        text = TR("FightWinString");

    auto capturefile =
        FileUtils::getInstance()->getWritablePath() + "/capture.png";

    if (!GameOverView::init("common/B6pic_win@3x.png", text, capturefile))
        return false;

    return true;
}

bool FightLoseView::init()
{
    Sound::getInstance()->playEffect("lose");
    int count = UserData::getInstance()->getIntegerForKey("FightScene:LOSE", 0);
    UserData::getInstance()->setIntegerForKey("FightScene:LOSE", count+1);

    int lose_cont = UserData::getInstance()->getIntegerForKey("FightScene:LOSE_CONT", 0);
    UserData::getInstance()->setIntegerForKey("FightScene:LOSE_CONT", lose_cont+1);
    UserData::getInstance()->setIntegerForKey("FightScene:WIN_CONT", 0);

    std::string text;
    if (lose_cont+1 >= 3) {
        UserData::getInstance()->setIntegerForKey("FightScene:LOSE_CONT", 0);
        int level = LevelMenu::getUserLevel();
        if (level <= 0) {
            text = TR("FightLoseString");
        } else {
            LevelMenu::setUserLevel(level-1);
            auto levelStr = LevelMenu::getUserLevelString();
            text = TR("FightContLoseString");
            if (text.find("%s") != std::string ::npos) {
                char buf[256];
                snprintf(buf, sizeof(buf), text.c_str(), levelStr.c_str());
                text = buf;
            }
        }
    } else
        text = TR("FightLoseString");

    auto capturefile =
        FileUtils::getInstance()->getWritablePath() + "/capture.png";

    if (!GameOverView::init("common/B6pic_lost@3x.png", text, capturefile))
        return false;

    return true;
}

bool FightDrawView::init()
{
    auto capturefile =
        FileUtils::getInstance()->getWritablePath() + "/capture.png";

    if (!GameOverView::init("common/B6pic_lost@3x.png", TR("FightDrawString"), capturefile))
        return false;

    Sound::getInstance()->playEffect("draw");
    int count = UserData::getInstance()->getIntegerForKey("FightScene:DRAW", 0);
    UserData::getInstance()->setIntegerForKey("FightScene:DRAW", count+1);

    return true;
}
