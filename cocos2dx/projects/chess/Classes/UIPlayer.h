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
	virtual bool init() override;
	CREATE_FUNC(UIPlayer);

	virtual void start(std::string fen) override;
	virtual void stop() override;
    void forceStop(bool stop);

	virtual void onRequest(std::string req, std::string args="",
                           std::function<void(bool)>callback=nullptr) override;
    virtual void onReply(std::string reply, std::string args="") override;

	bool onTouchBegan(Touch *touch, Event *unused);
	void onTouchMoved(Touch *touch, Event *unused);
	void onTouchEnded(Touch *touch, Event *event);
	void onTouchCancelled(Touch *touch, Event *event);

	void setBoard(Board *board) { _board = board; };

private:
	Piece *_selectedPiece;
	EventListenerTouchOneByOne * _touchListener;
	Board *_board;
    bool _forceStop;
};

#endif
