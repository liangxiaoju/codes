#ifndef __FIGHTSCENE_H__
#define __FIGHTSCENE_H__

#include "cocos2d.h"
#include "Board.h"
#include "GameLayer.h"
#include "UIPlayer.h"
#include "AIPlayer.h"
#include "NetPlayer.h"

USING_NS_CC;

class FightScene : public Scene
{
public:
	enum Role {
		UI,
		AI,
		NET,
	};

	virtual bool init(Role white, Role black, int level, std::string fen);
	static FightScene* create(Role white, Role black, int level, std::string fen=Board::START_FEN)
	{
		FightScene *pRet = new(std::nothrow) FightScene();
		if (pRet && pRet->init(white, black, level, fen))
		{
			pRet->autorelease();
			return pRet;
		}
		else
		{
			delete pRet;
			pRet = nullptr;
			return nullptr;
		}
	}

private:
	Role _roleWhite;
	Role _roleBlack;
	int _level;
	std::string _fen;
	Player *_playerWhite;
	Player *_playerBlack;
	GameLayer *_gameLayer;
	Board *_board;
};

#endif
