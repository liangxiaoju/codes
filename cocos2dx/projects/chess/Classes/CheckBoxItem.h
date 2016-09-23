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
        addChild(layout);

        RelativeLayoutParameter *param;

        param = RelativeLayoutParameter::create();
        param->setAlign(
            RelativeLayoutParameter::RelativeAlign::PARENT_LEFT_CENTER_VERTICAL);
        auto text = Text::create(content, "", 35);
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

#endif
