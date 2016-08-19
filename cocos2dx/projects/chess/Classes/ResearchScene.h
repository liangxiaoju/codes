#ifndef __RESEARCHSCENE_H__
#define __RESEARCHSCENE_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "Board.h"
#include "NumberButton.h"

USING_NS_CC;
using namespace cocos2d::ui;

class PiecePannel : public HBox
{
public:
    bool init(Piece::Side side)
    {
        if (!HBox::init())
            return false;

        struct _data {
            char symbol;
            int count;
        };
        std::vector<_data> datas;
        if (side == Piece::Side::WHITE)
            datas = {{'R', 2}, {'N', 2}, {'B', 2}, {'A', 2}, {'C', 2}, {'P', 5}};
        else
            datas = {{'r', 2}, {'n', 2}, {'b', 2}, {'a', 2}, {'c', 2}, {'p', 5}};

        for (auto &data : datas) {
            char symbol = data.symbol;
            int count = data.count;
            auto filename = Piece::symbolToFileName(symbol);
            auto button = NumberButton::create(filename, "", "", count, false);
            addChild(button);
            auto param = LinearLayoutParameter::create();
            param->setMargin(Margin(10, 0, 10, 0));
            button->setLayoutParameter(param);
            _symbolMap[symbol] = button;
            button->setTag(symbol);
            button->addClickEventListener([this](Ref *ref) {
                    select(dynamic_cast<NumberButton*>(ref));
                });
            auto size = getContentSize();
            auto bsize = button->getContentSize();
            setContentSize(Size(size.width+bsize.width+10*2, bsize.height));
        }

        return true;
    }
    static PiecePannel* create(Piece::Side side) {
        auto pRet = new(std::nothrow) PiecePannel();
        if (pRet && pRet->init(side)) {
            pRet->autorelease();
            return pRet;
        } else {
            delete pRet;
            return nullptr;
        }
    }
    void select(NumberButton *button)
    {
        unselect();
        _selected = button;
        auto s = Sprite::create("board/OOS.png");
        s->setPosition(button->getPosition());
        addChild(s, 0, "select");
    }
    void unselect()
    {
        removeChildByName("select");
        _selected = nullptr;
    }
    Piece *getSelectedPiece()
    {
        if (_selected == nullptr)
            return nullptr;

        return Piece::create(_selected->getTag());
    }
    void addPiece(Piece *piece)
    {
        auto symbol = piece->getSymbol();
        auto button = _symbolMap[symbol];
        if (button)
            button->incNumber();
    }
    void removePiece(Piece *piece)
    {
        auto symbol = piece->getSymbol();
        auto button = _symbolMap[symbol];
        if (button)
            button->decNumber();
    }
    void clear()
    {
        for (auto &kv : _symbolMap) {
            auto button = kv.second;
            if (button) {
                button->setNumber(0);
            }
        }
    }
    void reinit()
    {
        for (auto &kv : _symbolMap) {
            auto symbol = kv.first;
            auto button = kv.second;
            int count = 2;
            if (symbol == 'p' || symbol == 'P')
                count = 5;
            button->setNumber(count);
        }
    }
private:
    std::map<char, NumberButton*> _symbolMap;
    NumberButton *_selected;
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
};

class ResearchSceneL2 : public Scene
{
public:
    virtual bool init(std::string fen);
    static ResearchSceneL2 *create(std::string fen) {
        auto pRet = new (std::nothrow) ResearchSceneL2();
        if (pRet && pRet->init(fen)) {
            pRet->autorelease();
            return pRet;
        }
        CC_SAFE_DELETE(pRet);
        return nullptr;
    }
};

#endif
