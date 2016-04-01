#include "GameLayer.h"
#include "Board.h"
#include "UIPlayer.h"

bool GameLayer::init()
{
	if (!Layer::init())
		return false;

	auto visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	auto board = Board::create();
	auto boardSize = board->getContentSize();
	addChild(board);
	board->setPosition(origin.x + visibleSize.width/2, origin.y + visibleSize.height/2);
	board->setScale(visibleSize.width/boardSize.width);

	auto player1 = UIPlayer::create();
	player1->setBoard(board);
	addChild(player1);
	player1->setSide(Piece::Side::WHITE);

	auto l1 = new (std::nothrow) Player::Listener();
	l1->onMoved = CC_CALLBACK_1(GameLayer::onPlayerWhiteMoved, this);
	player1->setListener(l1);

	auto player2 = UIPlayer::create();
	player2->setBoard(board);
	addChild(player2);
	player2->setSide(Piece::Side::BLACK);

	auto l2 = new (std::nothrow) Player::Listener();
	l2->onMoved = CC_CALLBACK_1(GameLayer::onPlayerBlackMoved, this);
	player2->setListener(l2);

	player1->go(0);

	_playerWhite = player1;
	_playerBlack = player2;

	return true;
}

void GameLayer::onPlayerWhiteMoved(std::string mv)
{
	_playerBlack->go(0);
}

void GameLayer::onPlayerBlackMoved(std::string mv)
{
	_playerWhite->go(0);
}
