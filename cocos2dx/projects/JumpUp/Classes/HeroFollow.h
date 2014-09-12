#ifndef __HEROFOLLOW_H__
#define __HEROFOLLOW_H__

#include "cocos2d.h"

USING_NS_CC;

class HeroFollow : public Follow {
public:
    //bool initWithTarget(Node *followedNode, const Rect& rect = Rect::ZERO);
    static HeroFollow* create(Node *followedNode, const Rect& rect = Rect::ZERO);

private:
    void step(float dt) override;
};

#endif
