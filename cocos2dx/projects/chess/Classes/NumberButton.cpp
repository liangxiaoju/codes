#include "NumberButton.h"

NumberButton *NumberButton::create(const std::string& filename, int num)
{
    auto pRet = new (std::nothrow) NumberButton();
    if (pRet && pRet->init(filename, num)) {
        pRet->autorelease();
        return pRet;
    }
    CC_SAFE_DELETE(pRet);
    return nullptr;
}

bool NumberButton::init(const std::string& filename, int num)
{
    if (!Button::init(filename))
        return false;

    auto layout = RelativeBox::create();
    layout->setContentSize(getContentSize());
    addChild(layout);

    auto param = RelativeLayoutParameter::create();
    param->setAlign(RelativeLayoutParameter::RelativeAlign::PARENT_TOP_RIGHT);
    _mark = Button::create("FightSceneMenu/red.png");
    _mark->setTitleFontSize(15);
    _mark->setLayoutParameter(param);
    _mark->setTouchEnabled(false);
    layout->addChild(_mark);

    setNumber(num);

    return true;
}

void NumberButton::setNumber(int num)
{
    std::stringstream ss;
    ss << num;
    _mark->setTitleText(ss.str());
    _number = num;

    if (_number > 0) {
        setEnabled(true);
    } else {
        setEnabled(false);
    }
}

void NumberButton::incNumber()
{
    setNumber(_number+1);
}

void NumberButton::decNumber()
{
    setNumber(std::max(_number-1, 0));
}

void NumberButton::releaseUpEvent()
{
    Widget::releaseUpEvent();
    decNumber();
}
