#include "ResearchScene.h"
#include "BGLayer.h"
#include "ControlLayer.h"
#include "XQFile/XQJsonFile.h"
#include "UserData.h"
#include "UIPlayer.h"
#include "GameLayer.h"
#include "Localization.h"

bool ResearchScene::init()
{
    if (!Scene::init())
        return false;

    auto vsize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    auto bgLayer = BGLayer::create();
    addChild(bgLayer);

    _board = Board::createWithFen("4k4/9/9/9/9/9/9/9/9/4K4 r");
    auto bsize = _board->getContentSize();
    _board->setPosition(origin.x + vsize.width/2, origin.y + vsize.height/2);
    auto factor = vsize.width/bsize.width;
    _board->setScale(factor);
    addChild(_board);
    bsize = _board->getContentSize() * factor;

    _pannelTop = PiecePannel::create(Piece::Side::BLACK);
    _pannelTop->setScale(factor);
    auto psize = _pannelTop->getContentSize() * _pannelTop->getScaleX();
    _pannelTop->setAnchorPoint(Vec2(0.5, 0.5));
    _pannelTop->setPosition(
        Vec2(origin.x+vsize.width/2,
             origin.y+vsize.height/2+bsize.height/2+psize.height/2+10));
    addChild(_pannelTop, 1);
    _pannelBottom = PiecePannel::create(Piece::Side::WHITE);
    _pannelBottom->setScale(factor);
    _pannelBottom->setAnchorPoint(Vec2(0.5, 0.5));
    _pannelBottom->setPosition(
        Vec2(origin.x+vsize.width/2,
             origin.y+vsize.height/2-bsize.height/2-psize.height/2-10));
    addChild(_pannelBottom, 1);

    auto place = Button::create("button.png");
    place->setTitleText(TR("Full Pieces"));
    place->setTitleFontSize(35);
    place->addClickEventListener([this, place, origin, vsize, factor](Ref *ref) {
            if (place->getTitleText() == TR("Full Pieces")) {
                removeChild(_board);
                _board = Board::create();
                _board->setPosition(origin.x + vsize.width/2,
                                    origin.y + vsize.height/2);
                _board->setScale(factor);
                addChild(_board);
                _pannelTop->clear();
                _pannelBottom->clear();
                place->setTitleText(TR("Clear Pieces"));
            } else {
                removeChild(_board);
                _board = Board::createWithFen("4k4/9/9/9/9/9/9/9/9/4K4 r");
                _board->setPosition(origin.x + vsize.width/2,
                                    origin.y + vsize.height/2);
                _board->setScale(factor);
                addChild(_board);
                _pannelTop->reinit();
                _pannelBottom->reinit();
                place->setTitleText(TR("Full Pieces"));
            }
        });
    auto placeSize = place->getContentSize();
    place->setPosition(Vec2(origin.x+vsize.width-placeSize.width/2,
                           origin.y+placeSize.height/2));
    addChild(place);

    auto done = Button::create("button.png");
    done->setTitleText(TR("Done"));
    done->setTitleFontSize(35);
    done->addClickEventListener([this](Ref *ref) {
            std::string fen = _board->getFen();
            auto scene = ResearchSceneL2::create(fen);
            Director::getInstance()->replaceScene(scene);
        });
    auto dsize = done->getContentSize();
    done->setPosition(Vec2(origin.x+vsize.width-dsize.width/2-placeSize.width-30,
                           origin.y+dsize.height/2));
    addChild(done);

    auto back = Button::create("DifficultyScene/look_back.png");
    back->addClickEventListener([](Ref *ref){
            Director::getInstance()->popScene();
        });
    back->setZoomScale(0.1);
    back->setPosition(Vec2(100, vsize.height-100));
    addChild(back);

    _touchListener = EventListenerTouchOneByOne::create();
    _touchListener->onTouchBegan = CC_CALLBACK_2(ResearchScene::onTouchBegan, this);
    _touchListener->onTouchEnded = CC_CALLBACK_2(ResearchScene::onTouchEnded, this);
    _touchListener->setSwallowTouches(true);

    setOnEnterCallback([this]() {
            getEventDispatcher()->addEventListenerWithFixedPriority(
                _touchListener, -1);
        });
    setOnExitCallback([this]() {
            //not attach to node, so we have to remove it myself
            getEventDispatcher()->removeEventListener(_touchListener);
        });

    return true;
}

bool ResearchScene::onTouchBegan(Touch *touch, Event *event)
{
    Vec2 touchPos = touch->getLocation();

    Rect boardRect = _board->getBoundingBox();
    Rect topRect = _pannelTop->getBoundingBox();
    Rect bottomRect = _pannelBottom->getBoundingBox();

    if (boardRect.containsPoint(touchPos)) {
        log("Researchscene: touch board");
        Vec2 index = _board->convertWorldCoordToIndex(touchPos);
        Piece *p = _board->pick(index);
        Piece *topPiece = _pannelTop->getSelectedPiece();
        Piece *bottomPiece = _pannelBottom->getSelectedPiece();

        Piece *selectedPiece;
        if (topPiece)
            selectedPiece = topPiece;
        else if(bottomPiece)
            selectedPiece = bottomPiece;
        else
            selectedPiece = _selectedPiece;

        if (p && (p == selectedPiece)) {
            return true;
        }
        if (selectedPiece && p && (p->getSymbol() == 'k' || p->getSymbol() == 'K')) {
            _board->unselectAll();
            _pannelTop->unselect();
            _pannelBottom->unselect();
            _selectedPiece = nullptr;
            return true;
        }

        if (selectedPiece) {
            selectedPiece->retain();
            //remove selectedPiece at SRC
            if (_selectedPiece)
                _board->removePiece(_board->convertLocalCoordToIndex(
                                        _selectedPiece->getPosition()));
            if (topPiece)
                _pannelTop->removePiece(topPiece);
            if (bottomPiece)
                _pannelBottom->removePiece(bottomPiece);

            //move DST piece to pannel
            if (p != nullptr) {
                p->retain();
                _board->removePiece(index);
                if (p->getSide() == Board::Side::WHITE)
                    _pannelBottom->addPiece(p);
                else
                    _pannelTop->addPiece(p);
                p->release();
            }

            //place selectedPiece at DST
            _board->addPiece(index, selectedPiece);
            selectedPiece->release();

            _board->unselectAll();
            _pannelTop->unselect();
            _pannelBottom->unselect();
            _selectedPiece = nullptr;
        } else {
            _board->unselectAll();
            _board->select(index);
            _selectedPiece = p;
        }
        return true;
    } else if (topRect.containsPoint(touchPos)) {
        _pannelBottom->unselect();
        if (_selectedPiece) {
            if (_selectedPiece->getSide() == Piece::Side::BLACK) {
                _board->removePiece(_board->convertLocalCoordToIndex(
                                        _selectedPiece->getPosition()));
                _pannelTop->addPiece(_selectedPiece);
            }
            _board->unselectAll();
            _selectedPiece = nullptr;
            return true;
        }
        return false;
    } else if (bottomRect.containsPoint(touchPos)) {
        _pannelTop->unselect();
        if (_selectedPiece) {
            if (_selectedPiece->getSide() == Piece::Side::WHITE) {
                _board->removePiece(_board->convertLocalCoordToIndex(
                                        _selectedPiece->getPosition()));
                _pannelBottom->addPiece(_selectedPiece);
            }
            _board->unselectAll();
            _selectedPiece = nullptr;
            return true;
        }
        return false;
    } else {
        _board->unselectAll();
        _selectedPiece = nullptr;
        _pannelTop->unselect();
        _pannelBottom->unselect();
        return false;
    }

    return false;
}

void ResearchScene::onTouchEnded(Touch *touch, Event *event)
{
    if (_selectedPiece == nullptr) {
        Vec2 touchPos = touch->getStartLocation();
        Rect boardRect = _board->getBoundingBox();
        if (boardRect.containsPoint(touchPos)) {
            Vec2 index = _board->convertWorldCoordToIndex(touchPos);
            _board->unselect(index);
        }
    }
}


bool ResearchSceneL2::init(std::string fen)
{
    if (!Scene::init())
        return false;

    auto vsize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    auto bgLayer = BGLayer::create();
    addChild(bgLayer);

    auto board = Board::createWithFen(fen);
    auto playerWhite = UIPlayer::create();
    playerWhite->setBoard(board);
    auto playerBlack = UIPlayer::create();
    playerBlack->setBoard(board);
    auto gameLayer = GameLayer::create(playerWhite, playerBlack, board);
    addChild(gameLayer);

    auto control = ResearchControlLayer::create();
    addChild(control);

    auto save_cb = [fen, board, playerWhite, playerBlack, gameLayer](EventCustom *ev) {
        board->addComment("Done");
        auto xqFile = new XQJsonFile();
        auto header = xqFile->getHeader();
        header->title = "Research Create";
        header->fen = fen;
        for (auto &move : board->getHistoryMoves()) {
            XQNode *node = new XQNode();
            node->mv = Utils::toUcciMove(move.src, move.dst);
            node->comment = move.comment;
            xqFile->addStep(node);
        }
        std::string json = xqFile->save();
        delete xqFile;
        log("Research: %s", json.c_str());

        UserData::SaveElement e;
        e.type = TYPE_RESEARCH;
        e.roleWhite = 0;//UI
        e.roleBlack = 0;//UI
        e.content = json;
        UserData::getInstance()->insertSaveElement(e);
    };

    setOnEnterCallback([this, save_cb]() {
            getEventDispatcher()->addCustomEventListener(EVENT_SAVE, save_cb);
        });

    setOnExitCallback([this]() {
            getEventDispatcher()->removeCustomEventListeners(EVENT_SAVE);
        });

    return true;
}
