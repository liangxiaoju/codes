#include "NumberButton.h"

NumberButton *NumberButton::create(const std::string& normal,
                                   const std::string& selected,
                                   const std::string& disabled,
                                   int num, bool autoDec)
{
    auto pRet = new (std::nothrow) NumberButton();
    if (pRet && pRet->init(normal, selected, disabled, num, autoDec)) {
        pRet->autorelease();
        return pRet;
    }
    CC_SAFE_DELETE(pRet);
    return nullptr;
}

bool NumberButton::init(const std::string& normal,
                        const std::string& selected,
                        const std::string& disabled,
                        int num, bool autoDec)
{
    if (!Button::init(normal, selected, disabled))
        return false;

    _layout = RelativeBox::create();
    _layout->setContentSize(getContentSize());

    addChild(_layout);

    auto param = RelativeLayoutParameter::create();
    param->setAlign(RelativeLayoutParameter::RelativeAlign::PARENT_TOP_RIGHT);
    _mark = Button::create("FightSceneMenu/red.png");
    _mark->setTitleFontSize(15);
    _mark->setLayoutParameter(param);
    _mark->setTouchEnabled(false);
    _layout->addChild(_mark);

    setNumber(num);
    _autoDec = autoDec;

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
        _mark->setVisible(true);
    } else {
        setEnabled(false);
        _mark->setVisible(false);
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
    if (_autoDec)
        decNumber();
    //releaseUpEvent may delete this
    Widget::releaseUpEvent();
}
