#ifndef __BGLAYER_H__
#define __BGLAYER_H__

#include "cocos2d.h"

USING_NS_CC;

class BGLayer : public Layer {
public:
    virtual bool init();
    static BGLayer *create() {
        BGLayer *bg = new BGLayer();
        if (bg && bg->init())
        {
            bg->autorelease();
            return bg;
        }
        CC_SAFE_DELETE(bg);
        return nullptr;
    }

protected:
    void update(float dt) override;
};

#endif
