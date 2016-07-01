#ifndef __GAMELAYER_H__
#define __GAMELAYER_H__

#include "cocos2d.h"
#include "Player.h"
#include "Board.h"

USING_NS_CC;

class GameLayer : public Layer
{
public:
	bool init(Player *white, Player *black, Board *board);
	static GameLayer* create(Player *white, Player *black, Board *board) {
		GameLayer *pRet = new(std::nothrow) GameLayer();
		if (pRet && pRet->init(white, black, board)) {
			pRet->autorelease();
			return pRet;
		} else {
			delete pRet;
			pRet = nullptr;
			return nullptr;
		}
	}

	int onRequest(std::string req);

	virtual ~GameLayer();

private:
	void onPlayerWhiteMoveRequest(std::string mv);
	void onPlayerWhiteResignRequest();
	void onPlayerWhiteDrawRequest();
	void onPlayerWhiteRegretRequest();

	void onPlayerBlackMoveRequest(std::string mv);
	void onPlayerBlackResignRequest();
	void onPlayerBlackDrawRequest();
	void onPlayerBlackRegretRequest();

	Player *_playerWhite;
	Player *_playerBlack;
	Board *_board;
	Player::Delegate *_playerWhiteDelegate;
	Player::Delegate *_playerBlackDelegate;
};

#endif
