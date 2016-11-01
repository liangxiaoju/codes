#ifndef __PIECE_H__
#define __PIECE_H__

#include "cocos2d.h"

USING_NS_CC;

class Piece : public cocos2d::Sprite
{
public:
	enum class Role
	{
		KING,
		ADVISOR,
		BISHOP,
		KNIGHT,
		ROOK,
		CANNON,
		PAWN
	};

	enum class Side
	{
		WHITE,
		BLACK
	};

	static Piece *create(char c);
	static Piece *create(Side side, Role role);
	Role getRole() { return _role; };
	Side getSide() { return _side; };
	static Side symbolToSide(char c) { return _symbolMap.at(c).first; };
	static Role symbolToRole(char c) { return _symbolMap.at(c).second; };
    static std::string symbolToFileName(char c)
    {
        auto side = symbolToSide(c);
        auto role = symbolToRole(c);
        return _bitMapBaseDir + _bitMap.at(std::make_pair(side, role));
    }
	static char toSymbol(Side side, Role role)
	{
		for (auto &kv : _symbolMap) {
			if (kv.second == std::make_pair(side, role)) {
				return kv.first;
            }
		}
		return 0;
	}
	static char sideToChar(Side side)
	{
		return side == Side::BLACK ? 'b' : 'r';
	}
	static Side charToSide(char c)
	{
		return c == 'b' ? Side::BLACK : Side::WHITE;
	}

	char getSymbol() { return toSymbol(_side, _role); }
    Piece *clone();
    std::string getFileName() { return _filename; }

private:
	bool initWithSymbol(char c);
	bool initWithType(Side side, Role role);
	Role _role;
	Side _side;
    std::string _filename;

    static const std::string _bitMapBaseDir;
	static const std::map<std::pair<Side, Role>, std::string> _bitMap;
	static const std::map<char, std::pair<Side, Role>> _symbolMap;
};

#endif
