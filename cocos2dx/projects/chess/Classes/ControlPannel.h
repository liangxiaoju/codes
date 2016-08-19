#ifndef __CONTROLPANNEL_H__
#define __CONTROLPANNEL_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"

USING_NS_CC;
using namespace cocos2d::ui;

class ControlPannel : public RelativeBox
{
public:
    virtual bool init()
    {
        if (!RelativeBox::init())
            return false;

        setBackGroundImage("FightSceneMenu/game_button_F_bg_new.png");
        setBackGroundImageScale9Enabled(true);
        setBackGroundImageCapInsets(Rect(80, 50, 437-80-25, 154-50-25));
        setContentSize(Size(0, 154));

        auto param = RelativeLayoutParameter::create();
        param->setAlign(RelativeLayoutParameter::RelativeAlign::CENTER_IN_PARENT);

        _layout = HBox::create();
        _layout->setLayoutParameter(param);
        _layout->setContentSize(getContentSize());
        addChild(_layout);

        return true;
    }
    CREATE_FUNC(ControlPannel);

    virtual void addWidget(Widget *widget)
    {
        auto param = LinearLayoutParameter::create();
        param->setMargin(Margin(10, 30, 10, 30));
        widget->setLayoutParameter(param);

        _layout->addChild(widget);

        auto widgetSize = widget->getContentSize();
        auto layoutSize = _layout->getContentSize();

        float width = layoutSize.width + widgetSize.width + 20;
        float height = std::max(layoutSize.height, widgetSize.height);

        _layout->setContentSize(Size(width, height));
        setContentSize(Size(width+40, height));
    }

    virtual Node *getChildByName(const std::string& name) const
    {
        Node *child;

        child = Widget::getChildByName(name);
        if (child == nullptr)
            child = _layout->getChildByName(name);

        return child;
    }

    virtual void setEnabled(bool enable)
    {
        auto children = _layout->getChildren();
        for (auto &child : children) {
            auto widget = dynamic_cast<Widget*>(child);
            widget->setEnabled(enable);
        }
    }

private:
    Layout *_layout;
};

#endif
