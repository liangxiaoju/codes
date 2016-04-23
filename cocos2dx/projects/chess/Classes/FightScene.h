#ifndef __FIGHTSCENE_H__
#define __FIGHTSCENE_H__

#include "cocos2d.h"
#include "GameLayer.h"

USING_NS_CC;

class FightScene : public cocos2d::Scene
{
public:
	virtual bool init(GameLayer::Mode mode, Piece::Side side,
			int level, std::string fen);
	static FightScene* create(GameLayer::Mode mode, Piece::Side side,
			int level, std::string fen=Board::START_FEN)
	{
		FightScene *pRet = new(std::nothrow) FightScene();
		if (pRet && pRet->init(mode, side, level, fen))
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
};

#endif
