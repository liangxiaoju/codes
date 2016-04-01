#include "Board.h"

template<typename T>
std::string toString(T arg)
{
    std::stringstream ss;
    ss << arg;
    return ss.str();
}

const std::string Board::START_FEN =
	"rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR r";

bool Board::init()
{
	if (!Sprite::initWithFile("WOOD.png"))
		return false;

	auto contentSize = getContentSize();
	_stepX = contentSize.width/9;
	_stepY = contentSize.height/10;

	initFromFen(START_FEN);

	return true;
}

std::string Board::getFen()
{
	std::string fen = "";

	for (size_t r = 9; r >= 0; --r) {
		int n = 0;

		for (size_t c = 0; c <= 8; ++c) {
			auto it = _mapPieces.find(Vec2(c, r));
			if (it != _mapPieces.end()) {
				if (n != 0) {
					fen += toString(n);
					n = 0;
				}
				fen += (*it).second->getSymbol();
			} else {
				n += 1;
			}
		}
		if (n != 0)
			fen += toString(n);
		if (r != 0)
			fen += "/";
	}

	fen += " " + Piece::sideToChar(_currSide);

	return fen;
}

void Board::initFromFen(std::string fen)
{
	size_t r, c;
	std::string row;
	std::vector<std::string> rows;
	std::vector<std::string> substr;

	substr = splitString(fen, ' ');

	rows = splitString(substr[0], '/');
	for (r = 0; r < rows.size(); ++r)
	{
		c = 0;
		row = rows[r];
		for (auto it = row.begin(); it != row.end(); ++it) {
			if (isdigit(*it))
				c += *it - '0';
			else {
				/* reverse 9-r */
				addPiece(Vec2(c, 9-r), Piece::create(*it));
				c += 1;
			}
		}
	}

	_currSide = Piece::charToSide(substr[1][0]);
}

void Board::addPiece(Vec2 index, Piece *p)
{
	removePiece(index);

	p->setPosition(convertIndexToLocalCoord(index));
	addChild(p, 0, "piece");

	_mapPieces[index] = p;
}

void Board::removePiece(Vec2 index)
{
	auto it = _mapPieces.find(index);
	if (it != _mapPieces.end()) {
		removeChild((*it).second, true);
		_mapPieces.erase(index);
	}
}

Vec2 Board::convertIndexToLocalCoord(Vec2 index)
{
	Vec2 coord;

	if ((index.x < 0 || index.x > 9) || (index.y < 0 || index.y > 10))
		return Vec2::ZERO;

	coord.x = _stepX/2 + index.x * _stepX;
	coord.y = _stepY/2 + index.y * _stepY;
	return coord;
}

Vec2 Board::convertLocalCoordToIndex(Vec2 coord)
{
	auto boardSize = getContentSize();
    Rect rect(0, 0, boardSize.width, boardSize.height);
	Vec2 index;

	if (!rect.containsPoint(coord))
		return Vec2::ZERO;

	index.x = floorf(coord.x/_stepX);
	index.y = floorf(coord.y/_stepY);

	index.x = (index.x < 0) ? 0 : ((index.x > 9) ? 9 : index.x);
	index.y = (index.y < 0) ? 0 : ((index.y > 10) ? 10 : index.y);

	return index;
}

int Board::move(Vec2 src, Vec2 dst)
{
	auto it = _mapPieces.find(src);
	if (it == _mapPieces.end())
		return -1;

	Piece *p = (*it).second;
	p->retain();

	removePiece(dst);
	removePiece(src);
	addPiece(dst, p);

	unselectAll();
	unmarkMoveAll();

	markMove(src, dst);

	return 0;
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
	auto s = Sprite::create("WOOD/OOS.png");
	s->setColor(Color3B::RED);
	s->setPosition(convertIndexToLocalCoord(src));
	addChild(s);

	auto d = Sprite::create("WOOD/OOS.png");
	d->setColor(Color3B::RED);
	d->setPosition(convertIndexToLocalCoord(dst));
	addChild(d);

	_marked[src] = s;
	_marked[dst] = d;
}

void Board::unmarkMoveAll()
{
	for (auto &kv : _marked) {
		removeChild(kv.second, true);
	}
	_marked.clear();
}

void Board::select(Vec2 index)
{
	auto it = _selected.find(index);
	if (it != _selected.end())
		return;

	auto s = Sprite::create("WOOD/OOS.png");
	s->setPosition(convertIndexToLocalCoord(index));
	addChild(s);
	_selected[index] = s;
}

void Board::unselect(Vec2 index)
{
	auto it = _selected.find(index);
	if (it != _selected.end()) {
		removeChild((*it).second, true);
		_selected.erase(index);
	}
}

void Board::unselectAll()
{
	for (auto &kv : _selected) {
		removeChild(kv.second, true);
	}
	_selected.clear();
}

