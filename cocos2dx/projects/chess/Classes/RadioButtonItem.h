#ifndef __RADIOBUTTONITEM_H__
#define __RADIOBUTTONITEM_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"

USING_NS_CC;
using namespace cocos2d::ui;

class RadioButtonItem : public RadioButton
{
public:
    typedef std::function<void(bool selected)> RadioButtomItemCallback;
    static RadioButtonItem *create(std::string text,
                                   RadioButtomItemCallback cb = nullptr)
    {
        auto pRet = new (std::nothrow) RadioButtonItem();
        if (pRet && pRet->init(text, cb)) {
            pRet->autorelease();
            return pRet;
        }
        CC_SAFE_DELETE(pRet);
        return nullptr;
    }

    virtual bool init(std::string text, RadioButtomItemCallback cb)
    {
        if (!RadioButton::init("common/B3btn@3x.png",
                               "",
                               "common/B4btn_pressed@3x.png",
                               "",
                               ""))
            return false;

        setZoomScale(0.0f);

        auto size = getContentSize();

        auto t = Text::create(text, "", 60);
        t->setTextColor(Color4B(88, 49, 14, 255));
        t->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
        t->setPosition(Vec2(size.width/2, size.height/2));
        addChild(t);

        return true;
    }
};

#endif
