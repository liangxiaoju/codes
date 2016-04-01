#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "cocos2d.h"
#include "Board.h"

USING_NS_CC;

class Player : public Node
{
public:
	virtual bool init() { return true; }
	virtual void setBoard(Board *b) { _board = b; }
	virtual Board *getBoard() { return _board; }
	virtual void setSide(Piece::Side side) { _side = side; }
	virtual Piece::Side getSide() { return _side; }

	virtual void ponder() = 0;

	virtual void go(float timeout) = 0;

	class Listener : public Ref
	{
	public:
    	typedef std::function<void(std::string)> ccMovedCallback;
    	typedef std::function<void()> ccResignRequestCallback;
    	typedef std::function<void()> ccDrawRequestCallback;

		ccMovedCallback onMoved;
		ccResignRequestCallback onResignRequest;
		ccDrawRequestCallback onDrawRequest;
	};

	virtual void setListener(Listener *l) { _listener = l; }
	virtual Listener *getListener() { return _listener; }

private:
	Listener *_listener;
	Board *_board;
	Piece::Side _side;
};

#endif
