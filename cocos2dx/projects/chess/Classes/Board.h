#ifndef __BOARD_H__
#define __BOARD_H__

#include "cocos2d.h"
#include "Piece.h"

namespace std {
	template <>
		struct hash<cocos2d::Vec2>
		{
			std::size_t operator()(const cocos2d::Vec2& k) const
			{
				using std::size_t;
				using std::hash;
				return hash<float>()(k.x*10+k.y);
			}
		};
}

USING_NS_CC;

class Board : public Sprite
{
public:
	static const std::string START_FEN;

	enum class Style
	{
		DEFAULT,
	};

	virtual bool init() override;
	CREATE_FUNC(Board);


	Piece* pick(Vec2 index);

	int move(Vec2 src, Vec2 dst);

	void setStyle(Style s);

	std::string getFen();

	Vec2 convertIndexToLocalCoord(Vec2 index);
	Vec2 convertLocalCoordToIndex(Vec2 coord);

	Vec2 convertIndexToWorldCoord(Vec2 index)
	{
		return convertToWorldSpace(convertIndexToLocalCoord(index));
	}

	Vec2 convertWorldCoordToIndex(Vec2 coord)
	{
		return convertLocalCoordToIndex(convertToNodeSpace(coord));
	}

	void select(Vec2 index);
	void unselect(Vec2 index);
	void unselectAll();
	void markMove(Vec2 src, Vec2 dst);
	void unmarkMoveAll();

private:
	float _stepX;
	float _stepY;
	std::vector<std::string> _historyMv;
	std::map<Vec2, Sprite*> _marked;
	std::map<Vec2, Sprite*> _selected;
	std::vector<Vec2> _focused;
	std::vector<Piece> _pieces;
	std::unordered_map<Vec2, Piece*> _mapPieces;
	Style _style;
	Piece::Side _currSide;

	void addPiece(Vec2 index, Piece *p);
	void removePiece(Vec2 index);

	void initFromFen(std::string fen);

	std::vector<std::string> splitString(const std::string &s, char delim)
	{
		std::vector<std::string> elems;
		std::stringstream ss(s);
		std::string item;

		while (std::getline(ss, item, delim)) {
			elems.push_back(item);
		}

		return elems;
	}
};

#endif
