#ifndef __GAMESTATUSLAYER_H__
#define __GAMESTATUSLAYER_H__

#include "cocos2d.h"

USING_NS_CC;

class GameStatusLayer : public Layer {
public:
	virtual bool init();
	CREATE_FUNC(GameStatusLayer);

private:
	void onExit();
	void pauseCallback(Ref *sender);
	void addGameScore(int score);
	int mScore;
	Label *mScoreLabel;
	EventListenerCustom *mListener;
};

#endif
