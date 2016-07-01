#include "GameLayer.h"
#include "UIPlayer.h"
#include "AIPlayer.h"
#include "NetPlayer.h"
#include "UserData.h"

bool GameLayer::init(Player *white, Player *black, Board *board)
{
	if (!Layer::init())
		return false;

	auto vsize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	_playerWhite = white;
	_playerBlack = black;
	_board = board;

	auto boardSize = _board->getContentSize();
	_board->setPosition(origin.x + vsize.width/2, origin.y + vsize.height/2);
	_board->setScale(vsize.width/boardSize.width);

	addChild(_playerWhite);
	addChild(_playerBlack);
	addChild(_board);

	Player::Delegate *delegate;
	delegate = new (std::nothrow) Player::Delegate();
	delegate->onMoveRequest = CC_CALLBACK_1(GameLayer::onPlayerWhiteMoveRequest, this);
	delegate->onResignRequest = CC_CALLBACK_0(GameLayer::onPlayerWhiteResignRequest, this);
	delegate->onDrawRequest = CC_CALLBACK_0(GameLayer::onPlayerWhiteDrawRequest, this);
	delegate->onRegretRequest = CC_CALLBACK_0(GameLayer::onPlayerWhiteRegretRequest, this);
	_playerWhite->setDelegate(delegate);
	_playerWhiteDelegate = delegate;

	delegate = new (std::nothrow) Player::Delegate();
	delegate->onMoveRequest = CC_CALLBACK_1(GameLayer::onPlayerBlackMoveRequest, this);
	delegate->onResignRequest = CC_CALLBACK_0(GameLayer::onPlayerBlackResignRequest, this);
	delegate->onDrawRequest = CC_CALLBACK_0(GameLayer::onPlayerBlackDrawRequest, this);
	delegate->onRegretRequest = CC_CALLBACK_0(GameLayer::onPlayerBlackRegretRequest, this);
	_playerBlack->setDelegate(delegate);
	_playerBlackDelegate = delegate;

	setOnEnterCallback([this](){
		if (_board->getCurrentSide() == Board::Side::WHITE) {
			getEventDispatcher()->dispatchCustomEvent(EVENT_WHITE_START);
			_playerWhite->start(_board->getFenWithMove());
		} else {
			getEventDispatcher()->dispatchCustomEvent(EVENT_BLACK_START);
			_playerBlack->start(_board->getFenWithMove());
		}
	});

	return true;
}

GameLayer::~GameLayer()
{
	_playerWhite->stop();
	_playerBlack->stop();

	delete _playerWhiteDelegate;
	delete _playerBlackDelegate;

	log("### Delete GameLayer");
}

void GameLayer::onPlayerWhiteMoveRequest(std::string mv)
{
	log("WHITE: %s", mv.c_str());

	_playerWhite->stop();
	int retval = _board->move(mv);

	if (Rule::getInstance()->isMate(_board->getFen())) {
		getEventDispatcher()->dispatchCustomEvent(EVENT_GAMEOVER,
				(void *)"WIN:WHITE");
		getEventDispatcher()->dispatchCustomEvent(EVENT_WHITE_WIN);
	}

	switch (retval) {
	case 0:
		getEventDispatcher()->dispatchCustomEvent(EVENT_BLACK_START);
		_playerBlack->start(_board->getFenWithMove());
		break;
	case -3:
		onPlayerWhiteResignRequest();
		break;
	default:
		_playerWhite->start(_board->getFenWithMove());
	}
}

void GameLayer::onPlayerBlackMoveRequest(std::string mv)
{
	log("BLACK: %s", mv.c_str());

	_playerBlack->stop();
	int retval = _board->move(mv);

	if (Rule::getInstance()->isMate(_board->getFen())) {
		log("Black Win!");
		getEventDispatcher()->dispatchCustomEvent(EVENT_GAMEOVER,
				(void *)"WIN:BLACK");
		getEventDispatcher()->dispatchCustomEvent(EVENT_BLACK_WIN);
	}

	switch (retval) {
	case 0:
		getEventDispatcher()->dispatchCustomEvent(EVENT_WHITE_START);
		_playerWhite->start(_board->getFenWithMove());
		break;
	case -3:
		onPlayerBlackResignRequest();
		break;
	default:
		_playerBlack->start(_board->getFenWithMove());
	}
}

void GameLayer::onPlayerWhiteResignRequest()
{
	_playerWhite->stop();
	_playerBlack->stop();

	getEventDispatcher()->dispatchCustomEvent(EVENT_GAMEOVER,
			(void *)"WIN:BLACK");
	getEventDispatcher()->dispatchCustomEvent(EVENT_BLACK_WIN);
}

void GameLayer::onPlayerBlackResignRequest()
{
	_playerWhite->stop();
	_playerBlack->stop();

	getEventDispatcher()->dispatchCustomEvent(EVENT_GAMEOVER,
			(void *)"WIN:WHITE");
	getEventDispatcher()->dispatchCustomEvent(EVENT_WHITE_WIN);
}

void GameLayer::onPlayerWhiteDrawRequest()
{
	bool draw = _playerBlack->onRequest("draw");
	if (draw) {
		_playerWhite->stop();
		_playerBlack->stop();

		getEventDispatcher()->dispatchCustomEvent(EVENT_GAMEOVER,
				(void *)"DRAW:");
	}
}

void GameLayer::onPlayerBlackDrawRequest()
{
	bool draw = _playerWhite->onRequest("draw");
	if (draw) {
		_playerWhite->stop();
		_playerBlack->stop();

		getEventDispatcher()->dispatchCustomEvent(EVENT_GAMEOVER,
				(void *)"DRAW:");
	}
}

void GameLayer::onPlayerWhiteRegretRequest()
{
	if (_board->getCurrentSide() != Board::Side::WHITE)
		return;
	if (! _playerBlack->onRequest("regret"))
		return;
	_board->undo();
	_board->undo();
}

void GameLayer::onPlayerBlackRegretRequest()
{
	if (_board->getCurrentSide() != Board::Side::BLACK)
		return;
	if (! _playerWhite->onRequest("regret"))
		return;
	_board->undo();
	_board->undo();
	if (_board->getCurrentSide() != Board::Side::BLACK) {
		_playerBlack->stop();
		getEventDispatcher()->dispatchCustomEvent(EVENT_WHITE_START);
		_playerWhite->start(_board->getFenWithMove());
	}
}

int GameLayer::onRequest(std::string req)
{
	return 0;
}
