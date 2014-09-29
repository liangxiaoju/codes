#ifndef __HEROSPRITE_H__
#define __HEROSPRITE_H__

#include "cocos2d.h"

USING_NS_CC;

class HeroSprite : public Sprite {
public:
    virtual bool init() override;
    CREATE_FUNC(HeroSprite);

private:
    enum {
        STATE_IDLE = 1,
        STATE_JUMPUP,
        STATE_JUMPDOWN,
        STATE_STAND,
    };

    int mState;
    void setState(int state);

    void onEnter() override;
    void onExit() override;
	bool onContactBegin(PhysicsContact& contact);
    bool onContactPreSolve(PhysicsContact& contact, PhysicsContactPreSolve& solve);
	void onContactPostSolve(PhysicsContact& contact, const PhysicsContactPostSolve& solve);
	void onContactSeperate(PhysicsContact& contact);

protected:
    void update(float dt);
};

#endif
