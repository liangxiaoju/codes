#include "GameLayer.h"
#include "UIPlayer.h"
#include "AIPlayer.h"
#include "UserData.h"

bool GameLayer::init(GameLayer::Mode mode, Piece::Side uiSide,
		int level, std::string fen)
{
	if (!Layer::init())
		return false;

	auto visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	_board = Board::createWithFen(fen);
	auto boardSize = _board->getContentSize();
	addChild(_board);
	_board->setPosition(origin.x + visibleSize.width/2, origin.y + visibleSize.height/2);
	_board->setScale(visibleSize.width/boardSize.width);

	Player *player1, *player2;

	_mode = mode;
	_side = uiSide;
	_initFen = fen;

	if (mode == Mode::UI_TO_AI) {
		player1 = UIPlayer::create();
		player2 = AIPlayer::create();
	} else if (mode == Mode::UI_TO_UI) {
		player1 = UIPlayer::create();
		player2 = UIPlayer::create();
	} else if (mode == Mode::UI_TO_NET) {
		player1 = UIPlayer::create();
		player2 = AIPlayer::create();
	}

	if (uiSide == Piece::Side::WHITE) {
		_uiPlayer = _playerWhite = player1;
		_playerBlack = player2;
		auto s = _playerWhite->getContentSize();
		_playerWhite->setPosition(origin.x+visibleSize.width/2, origin.y+s.height/2+20);
		_playerBlack->setPosition(origin.x+visibleSize.width/2, origin.y+visibleSize.height-s.height/2-20);
	} else {
		_playerWhite = player2;
		_uiPlayer = _playerBlack = player1;
		auto s = _playerWhite->getContentSize();
		_playerBlack->setPosition(origin.x+visibleSize.width/2, origin.y+s.height/2+20);
		_playerWhite->setPosition(origin.x+visibleSize.width/2, origin.y+visibleSize.height-s.height/2-20);
		_board->setRotation(180);
	}

	_playerWhite->setBoard(_board);
	_playerBlack->setBoard(_board);
	_playerWhite->setSide(Piece::Side::WHITE);
	_playerBlack->setSide(Piece::Side::BLACK);
	addChild(_playerWhite);
	addChild(_playerBlack);

	auto playerWhiteListener = new (std::nothrow) Player::Listener();
	playerWhiteListener->onMoved = CC_CALLBACK_1(GameLayer::onPlayerWhiteMoved, this);
	playerWhiteListener->onResignRequest = CC_CALLBACK_0(GameLayer::onPlayerWhiteResignRequest, this);
	playerWhiteListener->onDrawRequest = CC_CALLBACK_0(GameLayer::onPlayerWhiteDrawRequest, this);
	_playerWhite->setListener(playerWhiteListener);

	auto playerBlackListener = new (std::nothrow) Player::Listener();
	playerBlackListener->onMoved = CC_CALLBACK_1(GameLayer::onPlayerBlackMoved, this);
	playerBlackListener->onResignRequest = CC_CALLBACK_0(GameLayer::onPlayerBlackResignRequest, this);
	playerBlackListener->onDrawRequest = CC_CALLBACK_0(GameLayer::onPlayerBlackDrawRequest, this);
	_playerBlack->setListener(playerBlackListener);

	setDifficulty(level);

	setOnEnterCallback([this](){
		if (_board->getCurrentSide() == Piece::Side::WHITE)
			_playerWhite->go(0);
		else
			_playerBlack->go(0);
	});

	return true;
}

GameLayer::~GameLayer()
{
	Player::Listener *l;
	l = _playerBlack->getListener();
	_playerBlack->setListener(nullptr);
	if (l != nullptr)
		delete l;
	l = _playerWhite->getListener();
	_playerWhite->setListener(nullptr);
	if (l != nullptr)
		delete l;

	_playerWhite->setBoard(nullptr);
	_playerBlack->setBoard(nullptr);

	log("### Delete GameLayer");
}

void GameLayer::onPlayerWhiteMoved(std::string mv)
{
	_playerWhite->stop();
	if (Rule::getInstance()->isMate(_board->getFen())) {
		log("Red Win!");
		getEventDispatcher()->dispatchCustomEvent(EVENT_GAMEOVER, (void *)"WIN:WHITE");
		getEventDispatcher()->dispatchCustomEvent(EVENT_WHITE_WIN);
		return;
	}
	getEventDispatcher()->dispatchCustomEvent(EVENT_BLACK_GO);
	_playerBlack->go(0);
}

void GameLayer::onPlayerBlackMoved(std::string mv)
{
	_playerBlack->stop();
	if (Rule::getInstance()->isMate(_board->getFen())) {
		log("Black Win!");
		getEventDispatcher()->dispatchCustomEvent(EVENT_GAMEOVER, (void *)"WIN:BLACK");
		getEventDispatcher()->dispatchCustomEvent(EVENT_BLACK_WIN);
		return;
	}
	getEventDispatcher()->dispatchCustomEvent(EVENT_WHITE_GO);
	_playerWhite->go(0);
}

void GameLayer::onPlayerWhiteResignRequest()
{
	_playerWhite->stop();
	_playerBlack->stop();

	getEventDispatcher()->dispatchCustomEvent(EVENT_GAMEOVER, (void *)"WIN:BLACK");
	getEventDispatcher()->dispatchCustomEvent(EVENT_BLACK_WIN);
}

void GameLayer::onPlayerBlackResignRequest()
{
	_playerWhite->stop();
	_playerBlack->stop();

	getEventDispatcher()->dispatchCustomEvent(EVENT_GAMEOVER, (void *)"WIN:WHITE");
	getEventDispatcher()->dispatchCustomEvent(EVENT_WHITE_WIN);
}

void GameLayer::onPlayerWhiteDrawRequest()
{
	bool draw = _playerBlack->askForDraw();
	if (draw) {
		_playerWhite->stop();
		_playerBlack->stop();

		getEventDispatcher()->dispatchCustomEvent(EVENT_GAMEOVER, (void *)"DRAW:");
		getEventDispatcher()->dispatchCustomEvent(EVENT_DRAW);
	}
}

void GameLayer::onPlayerBlackDrawRequest()
{
	bool draw = _playerWhite->askForDraw();
	if (draw) {
		_playerWhite->stop();
		_playerBlack->stop();

		getEventDispatcher()->dispatchCustomEvent(EVENT_GAMEOVER, (void *)"DRAW:");
		getEventDispatcher()->dispatchCustomEvent(EVENT_DRAW);
	}
}

void GameLayer::requestRegret()
{
	_playerWhite->stop();
	_playerBlack->stop();
	_board->undo();
	_board->undo();
	if (_board->getCurrentSide() == Piece::Side::WHITE)
		_playerWhite->go(0);
	else
		_playerBlack->go(0);
}

void GameLayer::requestLose()
{
	((UIPlayer*)_uiPlayer)->triggerLose();
}

void GameLayer::requestPeace()
{
	((UIPlayer*)_uiPlayer)->triggerPeace();
}

void GameLayer::setDifficulty(int level)
{
	_difficulty = level;
	if (_mode == Mode::UI_TO_AI) {
		if (_side == Piece::Side::WHITE) {
			((AIPlayer*)_playerBlack)->setDifficulty(level);
		} else {
			((AIPlayer*)_playerWhite)->setDifficulty(level);
		}
	}
}

void GameLayer::requestSave()
{
	UserData::SaveElement se;
	se.mode = (int)_mode;
	se.side = (int)_side;
	se.level = _difficulty;
	se.white = "white";
	se.black = "black";
	se.fen = _board->getFenWithMove();
	UserData::getInstance()->insertSaveElement(se);
}
