#ifndef __GAMELAYER_H__
#define __GAMELAYER_H__

#include "cocos2d.h"
#include "Player.h"
#include "Board.h"
#include "room/RoomManager.h"

USING_NS_CC;

class GameLayer : public cocos2d::Layer
{
public:
	enum class Mode
	{
		UI_TO_AI,
		UI_TO_UI,
		UI_TO_NET,
	};

	bool init(GameLayer::Mode mode, Piece::Side uiSide,
			int level, std::string fen);
	static GameLayer* create(
			GameLayer::Mode mode=Mode::UI_TO_AI,
			Piece::Side side=Piece::Side::WHITE,
			int level=0,
			std::string fen=Board::START_FEN)
	{
		GameLayer *pRet = new(std::nothrow) GameLayer();
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
	void requestRegret();
	void requestLose();
	void requestPeace();
	void requestSave();
	void setDifficulty(int level);
	int getDifficulty() { return _difficulty; }
	Mode getMode() { return _mode; }
	Piece::Side getSide() { return _side; }
	std::string getInitFen() { return _initFen; }
	virtual ~GameLayer();

	Player *getPlayer(Piece::Side side)
	{
		if (side == Piece::Side::WHITE)
			return _playerWhite;
		else
			return _playerBlack;
	}

private:
	void onPlayerWhiteMoved(std::string mv);
	void onPlayerBlackMoved(std::string mv);
	void onPlayerWhiteResignRequest();
	void onPlayerBlackResignRequest();
	void onPlayerWhiteDrawRequest();
	void onPlayerBlackDrawRequest();
	Player *_playerWhite;
	Player *_playerBlack;
	Player *_uiPlayer;
	Board *_board;
	Piece::Side _side;
	Mode _mode;
	int _difficulty;
	std::string _initFen;

	RoomServer *_server;
};

#endif
