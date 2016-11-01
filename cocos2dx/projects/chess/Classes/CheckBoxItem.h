#ifndef __CHECKBOXITEM_H__
#define __CHECKBOXITEM_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"

USING_NS_CC;
using namespace cocos2d::ui;

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
        layout->setBackGroundImage("common/B3btn@3x.png");
        layout->setContentSize(layout->getBackGroundImageTextureSize());
        addChild(layout);

        RelativeLayoutParameter *param;

        param = RelativeLayoutParameter::create();
        param->setAlign(
            RelativeLayoutParameter::RelativeAlign::PARENT_LEFT_CENTER_VERTICAL);
        auto text = Text::create(content, "", 60);
        text->setTextColor(Color4B(88, 49, 14, 255));
        text->setLayoutParameter(param);

        param = RelativeLayoutParameter::create();
        param->setAlign(
            RelativeLayoutParameter::RelativeAlign::PARENT_RIGHT_CENTER_VERTICAL);
        param->setMargin(Margin(0, 0, 60, 0));
        auto checkbox = CheckBox::create("common/B3radio_normal@3x.png",
                                         "common/B3radio_selected@3x.png");
        checkbox->setSelected(defaultVal);
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

        setContentSize(layout->getContentSize());

        return true;
    }
};

#endif
