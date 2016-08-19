#ifndef __NUMBERBUTTON_H__
#define __NUMBERBUTTON_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"

USING_NS_CC;
using namespace cocos2d::ui;

class NumberButton : public Button
{
public:
    static NumberButton *create(const std::string& normal,
                                const std::string& selected="",
                                const std::string& disabled="",
                                int num=0, bool autoDec=true);
    virtual bool init(const std::string& normal,
                      const std::string& selected,
                      const std::string& disabled,
                      int num, bool autoDec);
    virtual void setNumber(int num);
    virtual void incNumber();
    virtual void decNumber();

    virtual void releaseUpEvent() override;

private:
    int _number;
    bool _autoDec;
    Button *_mark;
    Layout *_layout;
};

#endif
