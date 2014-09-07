#ifndef __HEROSPRITE_H__
#define __HEROSPRITE_H__

#include "cocos2d.h"

USING_NS_CC;

class HeroSprite : public Sprite {
public:
	virtual bool init();
	CREATE_FUNC(HeroSprite);

private:
	enum {
		STATE_IDLE = 1,
		STATE_STAND,
		STATE_RUN,
		STATE_JUMP,
		STATE_JUMP1 = STATE_JUMP,
		STATE_JUMP2,
		STATE_DEAD,
	};

	int mState;
    FiniteTimeAction *mRunAnimate;
    FiniteTimeAction *mSmokeRunAnimate;
    FiniteTimeAction *mSmokeStandAnimate;

	void run();
	void jump();
	void stand();
	void dead();
	void onEnterTransitionDidFinish();
	void onKeyPressed(EventKeyboard::KeyCode keyCode, Event* unused_event);
	void onKeyReleased(EventKeyboard::KeyCode keyCode, Event* unused_event);
	bool onTouchBegan(Touch *touch, Event *event);
	void onTouchMoved(Touch *touch, Event *event);
	void onTouchEnded(Touch *touch, Event *event);
	bool onContactBegin(PhysicsContact& contact);
    bool onContactPreSolve(PhysicsContact& contact, PhysicsContactPreSolve& solve);
	void onContactPostSolve(PhysicsContact& contact, const PhysicsContactPostSolve& solve);
};

#endif
