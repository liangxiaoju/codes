#include "ResearchScene.h"
#include "BGLayer.h"
#include "FightControl.h"
#include "XQFile/XQJsonFile.h"
#include "UserData.h"
#include "UIPlayer.h"
#include "GameLayer.h"

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
    _board->setScale(vsize.width/bsize.width);
    addChild(_board);
    bsize = _board->getContentSize();

    _pannelTop = PiecePannel::create("2r2n2b2a2c5p");
    _pannelTop->setScale(vsize.width/bsize.width);
    auto psize = _pannelTop->getContentSize();
    _pannelTop->setPosition(origin.x+vsize.width/2,
                            origin.y+vsize.height/2+bsize.height/2+psize.height);
    addChild(_pannelTop);
    _pannelBottom = PiecePannel::create("2R2N2B2A2C5P");
    _pannelBottom->setScale(vsize.width/bsize.width);
    _pannelBottom->setPosition(origin.x+vsize.width/2,
                               origin.y+vsize.height/2-bsize.height/2-psize.height);
    addChild(_pannelBottom);

    _touchListener = EventListenerTouchOneByOne::create();
    _touchListener->onTouchBegan = CC_CALLBACK_2(ResearchScene::onTouchBegan, this);
    _touchListener->onTouchEnded = CC_CALLBACK_2(ResearchScene::onTouchEnded, this);
    getEventDispatcher()->addEventListenerWithSceneGraphPriority(_touchListener, this);

    _touchListener->setEnabled(true);

    auto done = Button::create("button.png");
    done->setTitleText("Done");
    done->setTitleFontSize(35);
    done->addClickEventListener([this](Ref *ref) {
            std::string fen = _board->getFen();
            auto scene = ResearchSceneL2::create(fen);
            Director::getInstance()->replaceScene(scene);
        });
    auto dsize = done->getContentSize();
    done->setPosition(Vec2(origin.x+vsize.width-dsize.width/2,
                           origin.y+dsize.height/2));
    addChild(done);

    return true;
}

bool ResearchScene::onTouchBegan(Touch *touch, Event *event)
{
    Vec2 touchPos = touch->getLocation();

    Rect boardRect = _board->getBoundingBox();
    Rect pannelTopRect = _pannelTop->getBoundingBox();
    Rect pannelBottomRect = _pannelBottom->getBoundingBox();

    if (boardRect.containsPoint(touchPos)) {
        log("Researchscene: touch board");
        Vec2 index = _board->convertWorldCoordToIndex(touchPos);
        Piece *p = _board->pick(index);
        if (p == _selectedPiece) {
            return true;
        }
        if (_selectedPiece != nullptr) {
            _selectedPiece->retain();
            //remove _selectedPiece at SRC
            if (_selectedIndex.y >= 0)
                _board->removePiece(_selectedIndex);
            _pannelTop->removePiece(_selectedPiece);
            _pannelBottom->removePiece(_selectedPiece);
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
            //place _selectedPiece at DST
            _board->addPiece(index, _selectedPiece);
            _selectedPiece->release();
            _selectedPiece = nullptr;
            _board->unselectAll();
        } else {
            _board->unselectAll();
            _board->select(index);
            _selectedPiece = p;
            _selectedIndex = index;
        }

    } else if (pannelTopRect.containsPoint(touchPos)) {
        log("ResearchScene: touch Top");
        Vec2 index = _pannelTop->convertWorldCoordToIndex(touchPos);
        Vec2 tmp = _pannelTop->convertToNodeSpace(touchPos);
        log("toIndex: (%f %f)->(%f %f)", tmp.x, tmp.y, index.x, index.y);
        _selectedPiece = _pannelTop->select(index);
        _selectedIndex = Vec2(index.x, -1);
    } else if (pannelBottomRect.containsPoint(touchPos)) {
        log("ResearchScene: touch Bottom");
        Vec2 index = _pannelBottom->convertWorldCoordToIndex(touchPos);
        Vec2 tmp = _pannelTop->convertToNodeSpace(touchPos);
        log("toIndex: (%f %f)->(%f %f)", tmp.x, tmp.y, index.x, index.y);
        _selectedPiece = _pannelBottom->select(index);
        _selectedIndex = Vec2(index.x, -1);
    } else
        return false;

    return true;
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

    auto control = FightControl::create();
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
