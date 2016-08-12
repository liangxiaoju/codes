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
    virtual bool onTouchBegan(Touch *touch, Event *event) override;
    virtual Rect getInnerBoundingBox();

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

class PopupMessage : public PopupBox
{
public:
    static PopupMessage* create(std::string text,
                                std::function<void(void)>cb=nullptr);
    virtual bool init(std::string text, std::function<void(void)>cb);
};

class PopupMenu : public PopupBox
{
public:
    static PopupMenu* create(std::vector<std::string> vec,
                             std::function<void(int index)>cb);
    virtual bool init(std::vector<std::string> vec,
                      std::function<void(int index)>cb);
};

#endif
