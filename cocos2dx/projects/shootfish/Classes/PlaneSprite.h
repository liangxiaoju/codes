#ifndef __PLANESPRITE_H__
#define __PLANESPRITE_H__

#include "cocos2d.h"
#include "BulletSprite.h"
#include "GameLayer.h"

USING_NS_CC;

class PlaneSprite : public Sprite {
public:
    virtual bool init();

    static PlaneSprite *create();
    static PlaneSprite *create(Point pos);

    void moveTo(float duration, Point position, int tag);
    void stop(int tag);

    void blowUp();

    void remove();

    virtual bool onTouchBegan(Touch *touch, Event *unused);
    virtual void onTouchMoved(Touch *touch, Event *unused);
    virtual void onTouchEnded(Touch *touch, Event *event);
    virtual void onTouchCancelled(Touch *touch, Event *event);
    virtual void onKeyPressed(EventKeyboard::KeyCode keyCode, Event *unused);
    virtual void onKeyReleased(EventKeyboard::KeyCode keyCode, Event *unused);

    virtual void onEnterTransitionDidFinish() override;

    void startShoot();
    void shootOnce(float dt);

    void superPower();

private:

    enum {
        RESERVE,
        LEFT,
        RIGHT,
        UP,
        DOWN,
    };

    int mSuperPower;
};

#endif
