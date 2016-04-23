#ifndef __BOARD_H__
#define __BOARD_H__

#include "cocos2d.h"
#include "Piece.h"
#include "Rule.h"
#include "Utils.h"

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

	bool initWithFen(std::string fen);
	static Board* createWithFen(std::string fen) {
		Board *pRet = new Board();
		if (pRet && pRet->initWithFen(fen)) {
			pRet->autorelease();
			return pRet;
		} else {
			delete pRet;
			pRet = NULL;
			return NULL;
		}
	}

	Piece* pick(Vec2 index);

	int move(Vec2 src, Vec2 dst);

	void setStyle(Style s);

	std::string getFen();
	std::string getFenWithMove();

	/* convert index to _pieceLayer's local coord */
	Vec2 convertIndexToLocalCoord(Vec2 index);
	/* convert _pieceLayer's local coord to index */
	Vec2 convertLocalCoordToIndex(Vec2 coord);

	Vec2 convertIndexToWorldCoord(Vec2 index)
	{
		return _pieceLayer->convertToWorldSpace(convertIndexToLocalCoord(index));
	}

	Vec2 convertWorldCoordToIndex(Vec2 coord)
	{
		return convertLocalCoordToIndex(_pieceLayer->convertToNodeSpace(coord));
	}

	void select(Vec2 index);
	void unselect(Vec2 index);
	void unselectAll();
	void markMove(Vec2 src, Vec2 dst);
	void unmarkMoveAll();
	int checkMove(Vec2 src, Vec2 dst);
	void undo();
    virtual void setRotation(float rotation);
	Piece::Side getCurrentSide() { return _currSide; }
	void changeSide() {
		_currSide = (_currSide==Piece::Side::BLACK) ?
			Piece::Side::WHITE : Piece::Side::BLACK;
	}

private:
	std::string _initFen;
	float _startX;
	float _startY;
	float _stepX;
	float _stepY;
	typedef std::pair<Vec2, Vec2> Move;
	std::vector<std::pair<Move, Piece*>> _historyMv;
	std::map<Vec2, Sprite*> _marked;
	std::map<Vec2, Sprite*> _selected;
	std::map<Vec2, Piece*> _mapPieces;
	Style _style;
	Piece::Side _currSide;
	Node *_pieceLayer;

	void addPiece(Vec2 index, Piece *p);
	void removePiece(Vec2 index);

	void initFromFen(std::string fen);
};

#endif
