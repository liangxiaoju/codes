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
	typedef Piece::Side Side;

	static const std::string START_FEN;

	enum class Style
	{
		DEFAULT,
	};

	virtual bool init() override;
	CREATE_FUNC(Board);

	bool initWithFen(std::string fen);
	static Board* createWithFen(std::string fen) {
		Board *pRet = new (std::nothrow) Board();
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
    void moveWithCallback(std::string mv, std::function<void()> cb);

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
	int checkMove(std::string mv);
	void undo();
    virtual void setRotation(float rotation) override;
	Side getCurrentSide() { return _currSide; }
	void changeSide() {
		_currSide = (_currSide==Side::BLACK) ?
			Side::WHITE : Side::BLACK;
	}

	void addPiece(Vec2 index, Piece *p);
	void removePiece(Vec2 index);
    void addShadow(Vec2 index);
    void removeShadow(Vec2 index);

    virtual ~Board();

    struct Move {
        Vec2 src;
        Vec2 dst;
        Piece *capture;
        std::string comment;
    };

    std::vector<Move> getHistoryMoves() { return _historyMv; }

    void addComment(std::string comment) {
        if (!_historyMv.empty()) {
            _historyMv[_historyMv.size()-1].comment = comment;
        }
    }

protected:
	std::string _initFen;
	float _startX;
	float _startY;
	float _stepX;
	float _stepY;

    std::vector<Move> _historyMv;

	std::map<Vec2, Sprite*> _marked;
	std::map<Vec2, Sprite*> _shadows;
	std::map<Vec2, Sprite*> _selected;
	std::vector<Sprite*> _tips;
	std::map<Vec2, Piece*> _mapPieces;
	Style _style;
	Side _currSide;
	Node *_pieceLayer;
    bool _enableShadow;

	void initFromFen(std::string fen);
};

#endif
