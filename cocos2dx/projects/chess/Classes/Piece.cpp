#include "Piece.h"

Piece* Piece::create(char c) {
    Piece *pRet = new Piece();
    if (pRet && pRet->initWithSymbol(c)) {
        pRet->autorelease();
        return pRet;
    } else {
        delete pRet;
        pRet = NULL;
        return NULL;
    }
}

Piece* Piece::create(Side side, Role role) {
    Piece *pRet = new Piece();
    if (pRet && pRet->initWithType(side, role)) {
        pRet->autorelease();
        return pRet;
    } else {
        delete pRet;
        pRet = NULL;
        return NULL;
    }
}

//c++11, for c++98 use static method to init static member
const std::map<char, std::pair<Piece::Side, Piece::Role>> Piece::_symbolMap = {
	{ 'k', std::make_pair(Piece::Side::BLACK, Piece::Role::KING) },
	{ 'a', std::make_pair(Piece::Side::BLACK, Piece::Role::ADVISOR) },
	{ 'b', std::make_pair(Piece::Side::BLACK, Piece::Role::BISHOP) },
	{ 'n', std::make_pair(Piece::Side::BLACK, Piece::Role::KNIGHT) },
	{ 'r', std::make_pair(Piece::Side::BLACK, Piece::Role::ROOK) },
	{ 'c', std::make_pair(Piece::Side::BLACK, Piece::Role::CANNON) },
	{ 'p', std::make_pair(Piece::Side::BLACK, Piece::Role::PAWN) },

	{ 'K', std::make_pair(Piece::Side::WHITE, Piece::Role::KING) },
	{ 'A', std::make_pair(Piece::Side::WHITE, Piece::Role::ADVISOR) },
	{ 'B', std::make_pair(Piece::Side::WHITE, Piece::Role::BISHOP) },
	{ 'N', std::make_pair(Piece::Side::WHITE, Piece::Role::KNIGHT) },
	{ 'R', std::make_pair(Piece::Side::WHITE, Piece::Role::ROOK) },
	{ 'C', std::make_pair(Piece::Side::WHITE, Piece::Role::CANNON) },
	{ 'P', std::make_pair(Piece::Side::WHITE, Piece::Role::PAWN) }
};

const std::map<std::pair<Piece::Side, Piece::Role>, std::string> Piece::_bitMap = {
	{ std::make_pair(Piece::Side::BLACK, Piece::Role::KING), "BK.png" },
	{ std::make_pair(Piece::Side::BLACK, Piece::Role::ADVISOR), "BA.png" },
	{ std::make_pair(Piece::Side::BLACK, Piece::Role::BISHOP), "BB.png" },
	{ std::make_pair(Piece::Side::BLACK, Piece::Role::KNIGHT), "BN.png" },
	{ std::make_pair(Piece::Side::BLACK, Piece::Role::ROOK), "BR.png" },
	{ std::make_pair(Piece::Side::BLACK, Piece::Role::CANNON), "BC.png" },
	{ std::make_pair(Piece::Side::BLACK, Piece::Role::PAWN), "BP.png" },

	{ std::make_pair(Piece::Side::WHITE, Piece::Role::KING), "RK.png" },
	{ std::make_pair(Piece::Side::WHITE, Piece::Role::ADVISOR), "RA.png" },
	{ std::make_pair(Piece::Side::WHITE, Piece::Role::BISHOP), "RB.png" },
	{ std::make_pair(Piece::Side::WHITE, Piece::Role::KNIGHT), "RN.png" },
	{ std::make_pair(Piece::Side::WHITE, Piece::Role::ROOK), "RR.png" },
	{ std::make_pair(Piece::Side::WHITE, Piece::Role::CANNON), "RC.png" },
	{ std::make_pair(Piece::Side::WHITE, Piece::Role::PAWN), "RP.png" }
};

bool Piece::initWithSymbol(char c)
{
	return initWithType(_symbolMap.at(c).first, _symbolMap.at(c).second);
}

bool Piece::initWithType(Side side, Role role)
{
	std::string filename;

	if (!Sprite::init())
		return false;

	_side = side;
	_role = role;

	filename = _bitMap.at(std::make_pair(side, role));
	setTexture("piece/WOOD/" + filename);

	//log("Piece: %s\n", filename.c_str());

	return true;
}
