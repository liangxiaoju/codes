#ifndef __RESEARCHSCENE_H__
#define __RESEARCHSCENE_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "Board.h"

USING_NS_CC;
using namespace cocos2d::ui;

class PiecePannel : public Node
{
public:
    bool init(std::string fen) {
        if (!Node::init())
            return false;

        setContentSize(Size(100*6, 100));
        _stepX = 100;
        _stepY = 100;
        setAnchorPoint(Vec2(0.5, 0.5));

        int n = 0;
        for (auto it = fen.begin(); it != fen.end(); ++it) {
            if (isdigit(*it))
                n = *it - '0';
            else {
                for (int i = 0; i < n; i++) {
                    auto p = Piece::create(*it);
                    if (p == nullptr)
                        break;
                    addPiece(p);
                }
            }
        }
        return true;
    }
    static PiecePannel* create(std::string fen) {
        PiecePannel *pRet = new(std::nothrow) PiecePannel();
        if (pRet && pRet->init(fen)) {
            pRet->autorelease();
            return pRet;
        } else {
            delete pRet;
            return nullptr;
        }
    }

    Piece *select(Vec2 index) {
        removeChildByName("select");
        int x = index.x;
        log("pannel: select %d", x);
        if (!_data[x].empty()) {
            auto p = _data[x].back();
            auto s = Sprite::create("board/OOS.png");
            s->setPosition(p->getPosition());
            addChild(s, 0, "select");
            return p;
        }
        return nullptr;
    }

    void unselect() {
        removeChildByName("select");
    }

    void addPiece(Piece *p) {
        unselect();
        char symbol = p->getSymbol();
        int index = convertSymbolToIndex(symbol);
        _data[index].push_back(p);
        p->setPosition(convertIndexToCoord(Vec2(index,0)));
        addChild(p);
        log("pannel: add %c to %d (%f %f)", symbol, index,
            p->getPositionX(), p->getPositionY());
    }

    void removePiece(Piece *p) {
        unselect();
        char symbol = p->getSymbol();
        int index = convertSymbolToIndex(symbol);
        log("pannel: remove %c from %d", symbol, index);
        auto v = &_data[index];
        auto iter = std::find(v->begin(), v->end(), p);
        if (iter != v->end()) {
            v->erase(iter);
            removeChild(p);
        }
    }

    Vec2 convertWorldCoordToIndex(Vec2 coord) {
        coord = this->convertToNodeSpace(coord);
        Vec2 index;

        index.x = floorf(coord.x/_stepX);
        index.y = 0;
        return index;
    }

    Vec2 convertIndexToCoord(Vec2 index) {
        Vec2 coord;
        coord.y = _stepY/2;
        coord.x = index.x * _stepX + _stepX/2;
        return coord;
    }

private:
    int convertSymbolToIndex(char symbol) {
        std::map<char,int> map = {
            {'r',0},{'n',1},{'b',2},{'a',3},{'c',4},{'p',5},
            {'R',0},{'N',1},{'B',2},{'A',3},{'C',4},{'P',5},
        };
        return map[symbol];
    }
    std::vector<Piece*> _data[6];
    float _stepX;
    float _stepY;
};

class ResearchScene : public Scene
{
public:
    virtual bool init();
    CREATE_FUNC(ResearchScene);
    bool onTouchBegan(Touch *touch, Event *unused);
    void onTouchEnded(Touch *touch, Event *event);

private:
    Board *_board;
    PiecePannel *_pannelTop;
    PiecePannel *_pannelBottom;
    EventListenerTouchOneByOne * _touchListener;
    Piece *_selectedPiece;
    Vec2 _selectedIndex;
};

#endif
