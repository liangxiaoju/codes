#ifndef __ADCONTROL_H__
#define __ADCONTROL_H__

#include "cocos2d.h"

USING_NS_CC;

class AdController {
public:
    static AdController *getInstance();
    void setVisible(bool visible);

private:
    static AdController *s_adController;
};

#endif
