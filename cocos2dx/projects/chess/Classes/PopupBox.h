#ifndef __POPUPBOX_H__
#define __POPUPBOX_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"

USING_NS_CC;
using namespace cocos2d::ui;

class PopupBox : public LayerColor
{
public:
    PopupBox();
    virtual ~PopupBox();
    static PopupBox* create();
    virtual bool init() override;
    virtual void pushBackView(Widget* child);

private:
    Layout* _layout;
    LinearLayoutParameter* _default_parameter;
};

class DialogBox : public PopupBox
{
public:
    static DialogBox* create(Text *text, Button *positive, Button *negative);
    static DialogBox* create(std::string text,
                             std::string positive,
                             std::string negative,
                             std::function<void(bool positive)> cb);

    virtual bool init(Text *text, Button *positive, Button *negative);
    virtual bool init(std::string text,
                      std::string positive,
                      std::string negative,
                      std::function<void(bool positive)> cb);
};

#endif
