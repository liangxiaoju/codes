#ifndef __NUMBERBUTTON_H__
#define __NUMBERBUTTON_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"

USING_NS_CC;
using namespace cocos2d::ui;

class NumberButton : public Button
{
public:
    static NumberButton *create(const std::string& filename, int num=0);
    virtual bool init(const std::string& filename, int num);
    virtual void setNumber(int num);
    virtual void incNumber();
    virtual void decNumber();

    virtual void releaseUpEvent() override;

private:
    int _number;
    Button *_mark;
};

#endif
