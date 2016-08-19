#include "SchoolScene.h"
#include "ControlLayer.h"
#include "PopupBox.h"
#include "Localization.h"

bool SchoolScene::init(XQFile *xqFile)
{
    if (!Scene::init())
        return false;

    auto vsize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    auto fen = xqFile->getInitFen();
    _xqFile = xqFile;

    auto bgLayer = BGLayer::create();
    addChild(bgLayer);

    _board = Board::createWithFen(fen);
    _playerWhite = UIPlayer::create();
    _playerBlack = UIPlayer::create();
    _playerWhite->setBoard(_board);
    _playerBlack->setBoard(_board);
    _playerWhite->forceStop(true);
    _playerBlack->forceStop(true);

    auto bsize = _board->getContentSize();
    auto width = vsize.width;
    auto height = (vsize.height-bsize.height)/2;
    auto scrollView = ScrollView::create();
    scrollView->setScrollBarEnabled(false);
    scrollView->setBounceEnabled(true);
    scrollView->setDirection(ScrollView::Direction::VERTICAL);
    scrollView->setContentSize(Size(width, height));
    scrollView->setInnerContainerSize(scrollView->getContentSize());
    scrollView->setPosition(Vec2(0, vsize.height-height));
    scrollView->jumpToTopLeft();
    addChild(scrollView);

    auto text = Text::create("", "", 35);
    // auto newline
    text->ignoreContentAdaptWithSize(true);
    text->setTextAreaSize(Size(width, 0));
    text->setAnchorPoint(Vec2(0,0));
    text->setTextHorizontalAlignment(TextHAlignment::CENTER);
    text->setTextColor(Color4B::GRAY);
    scrollView->addChild(text);

    auto gameLayer = GameLayer::create(_playerWhite, _playerBlack, _board);
    addChild(gameLayer);

    auto control = TutorialControlLayer::create();
    addChild(control);

    auto prev = Button::create("button.png");
    prev->setTitleText(TR("Prev Step"));
    prev->setTitleFontSize(35);
    prev->addClickEventListener([this, text, scrollView](Ref *ref) {
            XQNode *node = _xqFile->prevStep();
            if (node == nullptr)
                return;
            _board->undo();
            text->setString(node->comment);
            scrollView->setInnerContainerSize(text->getContentSize());
        });
    auto psize = prev->getContentSize();
    prev->setPosition(Vec2(origin.x+psize.width/2, origin.y+psize.height/2));

    auto next = Button::create("button.png");
    next->setTitleText(TR("Next Step"));
    next->setTitleFontSize(35);
    next->addClickEventListener([this, text, scrollView](Ref *ref) {
            auto runNext = [this, text, scrollView]() {
                XQNode *node = _xqFile->nextStep();
                if (node == nullptr)
                    return;
                std::string mv = node->mv;
                std::string comment = node->comment;
                log("MV: %s", mv.c_str());
                log("COMMENT: %s", comment.c_str());
                text->setString(comment);
                scrollView->setInnerContainerSize(text->getContentSize());
                if (_board->getCurrentSide() == Board::Side::WHITE) {
                    _playerWhite->onRequest("move", mv);
                } else {
                    _playerBlack->onRequest("move", mv);
                }
            };

            auto alts = _xqFile->getNextAlts();
            if (alts.size() > 1) {
                log("ALT(%d)", alts.size());
                std::vector<std::string> items;
                for (auto &alt : alts) {
                    auto mv = Utils::toVecMove(alt->mv);
                    auto piece = _board->pick(mv[0]);
                    if (piece == nullptr)
                        continue;
                    auto symbol = piece->getSymbol();
                    auto str = Utils::convertMoveToString(alt->mv, symbol);
                    items.push_back(str);
                }
                auto menu = PopupMenu::create(items, [this, runNext, alts](int index) {
                        _xqFile->selectNextAlt(alts[index]);
                        runNext();
                    });
            } else {
                runNext();
            }

        });
    auto nsize = next->getContentSize();
    next->setPosition(Vec2(origin.x+vsize.width-nsize.width/2, origin.y+nsize.height/2));

    addChild(prev);
    addChild(next);

    setOnEnterCallback([this]() {

        });

    setOnExitCallback([this]() {

        });

    return true;
}

SchoolScene::~SchoolScene()
{
	if (_xqFile)
		delete _xqFile;
}
