#ifndef __UIPLAYER_H__
#define __UIPLAYER_H__

#include "cocos2d.h"
#include "Player.h"
#include "Board.h"
#include "Piece.h"

USING_NS_CC;

class UIPlayer : public Player
{
public:
	virtual bool init();
	CREATE_FUNC(UIPlayer);

	virtual void ponder() override;
	virtual void go(float timeout) override;

	virtual bool onTouchBegan(Touch *touch, Event *unused);
	virtual void onTouchMoved(Touch *touch, Event *unused);
	virtual void onTouchEnded(Touch *touch, Event *event);
	virtual void onTouchCancelled(Touch *touch, Event *event);

private:
	Piece *_selectedPiece;
	EventListenerTouchOneByOne * _touchListener;
};

#endif
