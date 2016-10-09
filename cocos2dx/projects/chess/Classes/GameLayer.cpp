#include "GameLayer.h"
#include "UIPlayer.h"
#include "AIPlayer.h"
#include "NetPlayer.h"
#include "UserData.h"
#include "Sound.h"

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

    auto move_cb = [this]() {
        if (Rule::getInstance()->isChecked(_board->getFenWithMove()))
            Sound::getInstance()->playEffect("check");
        else if (Rule::getInstance()->isCaptured(_board->getFenWithMove()))
            Sound::getInstance()->playEffect("capture");
        else
            Sound::getInstance()->playEffect("move");

		getEventDispatcher()->dispatchCustomEvent(EVENT_BLACK_START);
        _playerBlack->start(_board->getFenWithMove());

        if (Rule::getInstance()->isMate(_board->getFen())) {
            std::string args = _board->getCurrentSide() == Board::Side::BLACK ?
			"WIN:WHITE" : "WIN:BLACK";
            getEventDispatcher()->dispatchCustomEvent(EVENT_GAMEOVER,
                                                      (void *)args.c_str());
        }
    };

	_playerWhite->stop();
	int retval = _board->checkMove(mv);
    if (retval == -3) {
        onPlayerWhiteResignRequest();
    } else if (retval < 0) {
        Sound::getInstance()->playEffect("illegal");
        _board->unselectAll();
		_playerWhite->start(_board->getFenWithMove());
    } else {
        _board->moveWithCallback(mv, move_cb);
    }
}

void GameLayer::onPlayerBlackMoveRequest(std::string mv)
{
	log("BLACK: %s", mv.c_str());

    auto move_cb = [this]() {
        if (Rule::getInstance()->isChecked(_board->getFenWithMove()))
            Sound::getInstance()->playEffect("check");
        else if (Rule::getInstance()->isCaptured(_board->getFenWithMove()))
            Sound::getInstance()->playEffect("capture");
        else
            Sound::getInstance()->playEffect("move");

		getEventDispatcher()->dispatchCustomEvent(EVENT_WHITE_START);
        _playerWhite->start(_board->getFenWithMove());

        if (Rule::getInstance()->isMate(_board->getFen())) {
            std::string args = (_board->getCurrentSide() == Board::Side::BLACK) ?
			"WIN:WHITE" : "WIN:BLACK";
            getEventDispatcher()->dispatchCustomEvent(EVENT_GAMEOVER,
                                                      (void *)args.c_str());
        }
    };

	_playerBlack->stop();
	int retval = _board->checkMove(mv);
    if (retval == -3) {
        onPlayerBlackResignRequest();
    } else if (retval < 0) {
        Sound::getInstance()->playEffect("illegal");
        _board->unselectAll();
		_playerBlack->start(_board->getFenWithMove());
    } else {
        _board->moveWithCallback(mv, move_cb);
    }
}

void GameLayer::onPlayerWhiteResignRequest()
{
	if (_board->getCurrentSide() != Board::Side::WHITE)
		return;

    /* ignore the resign request if it has been mated */
    if (Rule::getInstance()->isMate(_board->getFen()))
        return;

    auto cb = [this](bool reply) {
        _playerWhite->stop();
        _playerBlack->stop();

        getEventDispatcher()->dispatchCustomEvent(EVENT_GAMEOVER,
                                                  (void *)"WIN:BLACK");
    };
    _playerBlack->onRequest("resign", "", cb);
}

void GameLayer::onPlayerBlackResignRequest()
{
	if (_board->getCurrentSide() != Board::Side::BLACK)
		return;

    if (Rule::getInstance()->isMate(_board->getFen()))
        return;

    auto cb = [this](bool reply) {
        _playerWhite->stop();
        _playerBlack->stop();

        getEventDispatcher()->dispatchCustomEvent(EVENT_GAMEOVER,
                                                  (void *)"WIN:WHITE");
    };
    _playerWhite->onRequest("resign", "", cb);
}

void GameLayer::onPlayerWhiteDrawRequest()
{
	if (_board->getCurrentSide() != Board::Side::WHITE)
		return;

    auto cb = [this](bool reply) {
        if (reply) {
            _playerWhite->stop();
            _playerBlack->stop();

            _playerWhite->onReply("draw", "accept");

            getEventDispatcher()->dispatchCustomEvent(EVENT_GAMEOVER,
                                                      (void *)"DRAW:");
        } else {
            _playerWhite->onReply("draw", "deny");
            _playerWhite->start(_board->getFenWithMove());
        }
    };

    _playerBlack->onRequest("draw", "", cb);
}

void GameLayer::onPlayerBlackDrawRequest()
{
	if (_board->getCurrentSide() != Board::Side::BLACK)
		return;
    auto cb = [this](bool reply) {
        if (reply) {
            _playerWhite->stop();
            _playerBlack->stop();

            _playerBlack->onReply("draw", "accept");

            getEventDispatcher()->dispatchCustomEvent(EVENT_GAMEOVER,
                                                      (void *)"DRAW:");
        } else {
            _playerBlack->onReply("draw", "deny");
            _playerBlack->start(_board->getFenWithMove());
        }
    };
	_playerWhite->onRequest("draw", "", cb);
}

void GameLayer::onPlayerWhiteRegretRequest()
{
	if (_board->getCurrentSide() != Board::Side::WHITE)
		return;

    auto cb = [this](bool reply) {
        if (reply) {
            _board->undo();
            _board->undo();
            Sound::getInstance()->playEffect("undo");
            _playerWhite->onReply("regret", "accept");
        } else {
            _playerWhite->onReply("regret", "deny");
        }
    };
	_playerBlack->onRequest("regret", "", cb);
}

void GameLayer::onPlayerBlackRegretRequest()
{
	if (_board->getCurrentSide() != Board::Side::BLACK)
		return;

    auto cb = [this](bool reply) {
        if (reply) {
            _board->undo();
            _board->undo();
            Sound::getInstance()->playEffect("undo");

            if (_board->getCurrentSide() != Board::Side::BLACK) {
                _playerBlack->stop();
                getEventDispatcher()->dispatchCustomEvent(EVENT_WHITE_START);
                _playerWhite->start(_board->getFenWithMove());
            }
            _playerBlack->onReply("regret", "accept");
        } else {
            _playerBlack->onReply("regret", "deny");
        }
    };
	_playerWhite->onRequest("regret", "", cb);
}

int GameLayer::onRequest(std::string req)
{
	return 0;
}
