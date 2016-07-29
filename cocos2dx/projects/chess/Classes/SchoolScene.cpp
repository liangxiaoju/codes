#include "SchoolScene.h"
#include "ChallengeControl.h"
#include "PopupBox.h"

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

    auto gameLayer = GameLayer::create(_playerWhite, _playerBlack, _board);
    addChild(gameLayer);

    auto text = Text::create("", "", 35);
    auto bsize = _board->getContentSize();
    auto width = vsize.width;
    auto height = (vsize.height-bsize.height)/2;
    text->setContentSize(Size(width, height));
    text->setPosition(Vec2(vsize.width/2, vsize.height-height/2));
    addChild(text);

    auto control = ChallengeControl::create();
    addChild(control);

    auto prev = Button::create("button.png");
    prev->setTitleText("Prev");
    prev->setTitleFontSize(35);
    prev->addClickEventListener([this](Ref *ref) {
            XQNode *node = _xqFile->prevStep();
            if (node == nullptr)
                return;
            _board->undo();
        });
    auto psize = prev->getContentSize();
    prev->setPosition(Vec2(origin.x+psize.width/2, origin.y+psize.height/2));

    auto next = Button::create("button.png");
    next->setTitleText("Next");
    next->setTitleFontSize(35);
    next->addClickEventListener([this, text](Ref *ref) {
            auto runNext = [this, text]() {
                XQNode *node = _xqFile->nextStep();
                if (node == nullptr)
                    return;
                std::string mv = node->mv;
                std::string comment = node->comment;
                log("MV: %s", mv.c_str());
                log("COMMENT: %s", comment.c_str());
                text->setString(comment);
                if (_board->getCurrentSide() == Board::Side::WHITE) {
                    _playerWhite->onRequest(std::string("tip:") + mv);
                } else {
                    _playerBlack->onRequest(std::string("tip:") + mv);
                }
            };

            auto v = _xqFile->getNextAlts();
            if (v.size() > 1) {
                log("ALT(%d)", v.size());
                auto box = PopupBox::create();
                auto b = Button::create("button.png");
                auto button_cb = [this, v, runNext, box](Ref *ref) {
                    auto b1 = dynamic_cast<Button*>(ref);
                    _xqFile->selectNextAlt(v[b1->getTag()]);
                    removeChild(box);
                    runNext();
                };
                b->setTitleFontSize(35);
                b->addClickEventListener(button_cb);
                for (int i = 0; i < v.size(); i++) {
                    Button* b1 = (Button*)b->clone();
                    b1->setTitleText(v[i]->mv);
                    box->pushBackView(b1);
                    b1->setTag(i);
                }
                addChild(box);
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
}
