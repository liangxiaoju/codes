#ifndef __CONTROLPANNEL_H__
#define __CONTROLPANNEL_H__

class ControlPannel : public Widget
{
public:
    virtual bool init()
    {
        if (!Layer::init())
            return false;

        _layout = HBox::create();
        addChild(_layout);

        _layout->setBackGroundImage("FightSceneMenu/game_button_bg_new.png");
        _layout->setBackGroundImageScale9Enabled(true);
        _layout->setBackGroundImageCapInsets(Rect(25, 45, 425-25, 154-25-45));

        return true;
    }
    virtual void addWidget(Widget *widget)
    {
        auto param = LinearLayoutParameter::create();
        param->setMargin(Margin(25, 0, 25, 0));
        widget->setLayoutParameter(param);

        _layout->addChild(widget);

        auto widgetSize = widget->getContentSize();
        auto layoutSize = _layout->getContentSize();

        float width = layoutSize.width + 25 + widgetSize.width + 25;
        float height = std::max(layoutSize.height, widgetSize.height+50);

        _layout->setContentSize(Size(width, height));
        setContentSize(Size(width, height));
    }

private:
    Layout *_layout;
};

#endif
