#include "Board.h"
#include "Utils.h"
#include "SettingMenu.h"

const std::string Board::START_FEN =
	"rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR r";

bool Board::initWithFen(std::string fen)
{
	if (!Sprite::initWithFile("chessboard.png"))
		return false;

	auto contentSize = getContentSize();
	_stepX = (contentSize.width-50-50)/8;
	_stepY = _stepX;

	_startX = _stepX/2+15;
	_startY = _stepY/2+35;

	_pieceLayer = Node::create();
	_pieceLayer->setContentSize(Size(_stepX*8, _stepY*9));
	_pieceLayer->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
	_pieceLayer->setPosition(_startX+_stepX*8/2, _startY+_stepY*9/2);
	addChild(_pieceLayer);

	initFromFen(fen);

	return true;
}

bool Board::init()
{
	return initWithFen(START_FEN);
}

std::string Board::getFen()
{
	std::string fen = "";

	for (int r = 9; r >= 0; --r) {
		int n = 0;

		for (int c = 0; c <= 8; ++c) {
			auto it = _mapPieces.find(Vec2(c, r));
			if (it != _mapPieces.end()) {
				if (n != 0) {
					fen += Utils::toString(n);
					n = 0;
				}
				fen += (*it).second->getSymbol();
			} else {
				n += 1;
			}
		}
		if (n != 0)
			fen += Utils::toString(n);
		if (r != 0)
			fen += "/";
	}

	fen = fen + " " + Piece::sideToChar(_currSide);

	return fen;
}

std::string Board::getFenWithMove()
{
	std::string fen = _initFen;
	std::string moves = "";

	if (fen.empty())
		return getFen();

	if (_historyMv.empty())
		return fen;

	for (auto &move : _historyMv) {
        auto ucciMv = Utils::toUcciMove(move.src, move.dst);
        moves = moves + " " + ucciMv;
	}

	return fen + " moves" + moves;
}

void Board::initFromFen(std::string fen)
{
	size_t r, c;
	std::string row;
	std::vector<std::string> rows;
	std::vector<std::string> substr;

	substr = Utils::splitString(fen, ' ');

	rows = Utils::splitString(substr[0], '/');
	for (r = 0; r < rows.size(); ++r)
	{
		c = 0;
		row = rows[r];
		for (auto it = row.begin(); it != row.end(); ++it) {
			if (isdigit(*it))
				c += *it - '0';
			else {
				/* reverse 9-r */
                auto p = Piece::create(*it);
                if (p == nullptr)
                    break;
				addPiece(Vec2(c, 9-r), p);
				c += 1;
			}
		}
	}

	_currSide = Piece::charToSide(substr[1][0]);

	_initFen = getFen();

	size_t i, n;
	if ((n = fen.find(" moves ")) != std::string::npos) {
		std::string mvstr = fen.substr(n+7);
		std::vector<std::string> mvs = Utils::splitString(mvstr, ' ');
		for (i = 0; i < mvs.size(); ++i) {
			std::vector<Vec2> mv = Utils::toVecMove(mvs[i]);
			move(mv[0], mv[1]);
		}
	}
}

void Board::addPiece(Vec2 index, Piece *p)
{
	removePiece(index);

	p->setPosition(convertIndexToLocalCoord(index));
	std::string name = Utils::toString(p->getSymbol());
	_pieceLayer->addChild(p, 0, name);

	_mapPieces[index] = p;
}

void Board::removePiece(Vec2 index)
{
	auto it = _mapPieces.find(index);
	if (it != _mapPieces.end()) {
		_pieceLayer->removeChild((Piece *)((*it).second), true);
		_mapPieces.erase(index);
	}
}

Vec2 Board::convertIndexToLocalCoord(Vec2 index)
{
	Vec2 coord;

	if ((index.x < 0 || index.x > 9) || (index.y < 0 || index.y > 10))
		return Vec2::ZERO;

	coord.x = index.x * _stepX;
	coord.y = index.y * _stepY;
	//coord.x = _startX + index.x * _stepX;
	//coord.y = _startY + index.y * _stepY;
	return coord;
}

Vec2 Board::convertLocalCoordToIndex(Vec2 coord)
{
	auto boardSize = getContentSize();
    Rect rect(0, 0, boardSize.width, boardSize.height);
	Vec2 index;

	coord.x = coord.x + _stepX/2;
	coord.y = coord.y + _stepY/2;
	//coord.x = coord.x - _startX + _stepX/2;
	//coord.y = coord.y - _startY + _stepY/2;

	if (!rect.containsPoint(coord))
		return Vec2::ZERO;

	index.x = floorf(coord.x/_stepX);
	index.y = floorf(coord.y/_stepY);

	index.x = (index.x < 0) ? 0 : ((index.x > 9) ? 9 : index.x);
	index.y = (index.y < 0) ? 0 : ((index.y > 10) ? 10 : index.y);

	return index;
}

int Board::checkMove(std::string mv)
{
    if (mv.substr(0, 2) == mv.substr(2, 2))
        return -1;

    auto vecs = Utils::toVecMove(mv);
    auto src = vecs[0];
    auto dst = vecs[1];

	auto it = _mapPieces.find(src);
	if (it == _mapPieces.end())
		return -1;

	std::string fen = getFenWithMove();

	if (fen.find(" moves ") != std::string::npos) {
		fen = fen + " " + Utils::toUcciMove(src, dst);
	} else {
		fen = fen + " moves " + Utils::toUcciMove(src, dst);
	}

	return Rule::getInstance()->check(fen);
}

int Board::move(Vec2 src, Vec2 dst)
{
	auto it = _mapPieces.find(src);
	if (it == _mapPieces.end())
		return -1;

	unselectAll();

	Piece *p = (*it).second;
	p->retain();

	auto dp = _mapPieces[dst];
	if (dp != nullptr) {
		dp->retain();
	}
    _historyMv.push_back({src, dst, dp});

	removePiece(dst);
	removePiece(src);
	addPiece(dst, p);
	p->release();

	unmarkMoveAll();
	markMove(src, dst);

	changeSide();

	return 0;
}

void Board::moveWithCallback(std::string mv, std::function<void()> cb)
{
    auto vecs = Utils::toVecMove(mv);
    auto src = vecs[0];
    auto dst = vecs[1];

	auto it = _mapPieces.find(src);
	if (it == _mapPieces.end())
		return;

	unselectAll();

	Piece *p = (*it).second;
	p->retain();

	auto dp = _mapPieces[dst];
	if (dp != nullptr) {
		dp->retain();
	}
    _historyMv.push_back({src, dst, dp});

    _mapPieces.erase(src);
    _mapPieces[dst] = p;
    changeSide();

    unmarkMoveAll();
    markMove(src, dst);

    auto moveTo = MoveTo::create(0.15, convertIndexToLocalCoord(dst));
    auto callback = CallFunc::create([this, p, dp, cb]() {
            if (dp != nullptr)
                _pieceLayer->removeChild(dp, true);
            p->release();
            cb();
        });
    auto seq = Sequence::create(moveTo, callback, nullptr);
    p->runAction(seq);
}

void Board::undo()
{
	if (_historyMv.empty())
		return;

    auto move = _historyMv.back();
    Vec2 src = move.src;
    Vec2 dst = move.dst;
    Piece *p = move.capture;

	unselectAll();

	auto dp = _mapPieces[dst];
	dp->retain();
	removePiece(dst);
	addPiece(src, dp);
	dp->release();

	if (p != nullptr) {
		addPiece(dst, p);
		p->release();
	}

	unmarkMoveAll();

	_historyMv.pop_back();

	changeSide();

	if (!_historyMv.empty()) {
        auto move = _historyMv.back();
        Vec2 src = move.src;
        Vec2 dst = move.dst;
        markMove(src, dst);
	}
}

Piece* Board::pick(Vec2 index)
{
	auto it = _mapPieces.find(index);
	if (it == _mapPieces.end())
		return nullptr;

	return (*it).second;
}

void Board::markMove(Vec2 src, Vec2 dst)
{
	auto s = Sprite::create("board/OOS.png");
	s->setColor(Color3B::RED);
	s->setPosition(convertIndexToLocalCoord(src));
	_pieceLayer->addChild(s);

	auto d = Sprite::create("board/OOS.png");
	d->setColor(Color3B::GREEN);
	d->setPosition(convertIndexToLocalCoord(dst));
	_pieceLayer->addChild(d);

	_marked[src] = s;
	_marked[dst] = d;
}

void Board::unmarkMoveAll()
{
	for (auto &kv : _marked) {
		_pieceLayer->removeChild(kv.second, true);
	}
	_marked.clear();
}

void Board::select(Vec2 index)
{
	auto it = _selected.find(index);
	if (it != _selected.end())
		return;

	auto s = Sprite::create("board/OOS.png");
	s->setPosition(convertIndexToLocalCoord(index));
	_pieceLayer->addChild(s);
	_selected[index] = s;

    if (SettingMenu::getInstance()->isTipsEnabled()) {
        std::vector<std::string> mvs =
            Rule::getInstance()->generateMoves(getFenWithMove(), index);

        for (auto &mv : mvs) {
            auto dst = Utils::toVecMove(mv)[1];
            auto s = Sprite::create("FightSceneMenu/red.png");
            s->setPosition(convertIndexToLocalCoord(dst));
            _pieceLayer->addChild(s);
            _tips.push_back(s);
        }
    }
}

void Board::unselect(Vec2 index)
{
	auto it = _selected.find(index);
	if (it != _selected.end()) {
		_pieceLayer->removeChild((*it).second, true);
		_selected.erase(index);
	}

	for (auto &p : _tips) {
		_pieceLayer->removeChild(p, true);
	}
	_tips.clear();
}

void Board::unselectAll()
{
	for (auto &kv : _selected) {
		_pieceLayer->removeChild(kv.second, true);
	}
	_selected.clear();

	for (auto &p : _tips) {
		_pieceLayer->removeChild(p, true);
	}
	_tips.clear();
}

void Board::setRotation(float rotation)
{
	_pieceLayer->setRotation(rotation);
	for (auto &kv: _mapPieces) {
		Piece *p = kv.second;
		p->setRotation(-rotation);
	}
}

Board::~Board()
{
    for (auto &move : _historyMv) {
        Piece *p = move.capture;
        if (p)
            p->release();
    }
}
