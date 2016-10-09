#include "TutorialScene.h"
#include "TutorialData.h"
#include "XQFile/XQJsonFile.h"
#include "ControlLayer.h"
#include "PopupBox.h"
#include "Localization.h"

bool TutorialScene::init(XQFile *xqFile)
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

    return true;
}

TutorialScene::~TutorialScene()
{
	if (_xqFile)
		delete _xqFile;
}


bool TutorialMenuScene::init()
{
    if (!Scene::init())
        return false;

    auto vsize = Director::getInstance()->getVisibleSize();
    LinearLayoutParameter* lp = LinearLayoutParameter::create();
    lp->setGravity(LinearLayoutParameter::LinearGravity::CENTER_HORIZONTAL);

    _category_button = Button::create("ChallengeScene/tga_title_fat.png");
    _category_button->setZoomScale(0.1);
    _category_button->setTitleFontSize(40);
    _category_button->setTitleFontName("");
    _category_button->retain();
    _node_button = Button::create("ChallengeScene/buttonPost.png");
    _node_button->setZoomScale(0.1);
    _node_button->setTitleFontSize(40);
    _node_button->setTitleFontName("");
    _node_button->retain();

    auto hideLoading = [this]() {
        removeChildByName("outter");
        removeChildByName("inner");
    };

    auto showLoading = [this]() {
        auto vsize = Director::getInstance()->getVisibleSize();
        RotateBy *rotateBy = RotateBy::create(0.1, -15);
        RepeatForever *repeat = RepeatForever::create(rotateBy);
        auto sp1 = Sprite::create("head/load_outter.png");
        sp1->runAction(repeat);
        sp1->setPosition(vsize.width/2, vsize.height/2);
        addChild(sp1, 0, "outter");
        auto sp2 = Sprite::create("load_inner.png");
        sp2->setPosition(vsize.width/2, vsize.height/2);
        addChild(sp2, 0, "inner");
    };

    auto refresh = [this, showLoading, hideLoading](int id) {
        struct QueryData {
            std::vector<TutorialData::TutorialNode>nodes;
            std::vector<TutorialData::TutorialCategory>categorys;
        };

        auto updateUI= [this, hideLoading](QueryData data) {
            auto nodes = data.nodes;
            auto categorys = data.categorys;

            for (auto &node : nodes) {
                Button* btn = (Button*)_node_button->clone();
                btn->setName("Node");
                btn->setTag(node.id);
                btn->setTitleText(node.name);
                _listview->pushBackCustomItem(btn);
                log("Node: %s", node.name.c_str());
            }

            for (auto &category : categorys) {
                Button* btn = (Button*)_category_button->clone();
                btn->setName("Category");
                btn->setTag(category.id);
                btn->setTitleText(category.name);
                _listview->pushBackCustomItem(btn);
                log("Category: %s", category.name.c_str());
            }

            hideLoading();
            this->release();
        };
        auto queryData = [this, id, updateUI]() {
            std::vector<TutorialData::TutorialCategory> categorys;
            TutorialData::getInstance()->queryTutorialCategory(id, categorys);
            std::vector<TutorialData::TutorialNode> nodes;
            TutorialData::getInstance()->queryTutorialNode(id, nodes);
            QueryData data = {nodes, categorys};

            getScheduler()->performFunctionInCocosThread([updateUI, data](){ updateUI(data); });
        };

        _listview->removeAllItems();

        this->retain();
        auto thread = std::thread([queryData]{ queryData(); });
        thread.detach();

        showLoading();
    };

    _listview = ListView::create();
    _listview->setDirection(ScrollView::Direction::VERTICAL);
    _listview->setBounceEnabled(true);
    _listview->setScrollBarEnabled(false);
    _listview->setGravity(ListView::Gravity::CENTER_HORIZONTAL);
    _listview->setItemsMargin(50);

    _listview->addEventListener([this, showLoading, hideLoading](Ref *pSender, ListView::EventType type){
    switch (type)
    {
        case ListView::EventType::ON_SELECTED_ITEM_START:
            {
                ListView* listView = static_cast<ListView*>(pSender);
                auto item = listView->getItem(listView->getCurSelectedIndex());
                CCLOG("select child start index = %d", item->getTag());
                break;
            }
        case ListView::EventType::ON_SELECTED_ITEM_END:
            {
                ListView* listView = static_cast<ListView*>(pSender);
                auto item = listView->getItem(listView->getCurSelectedIndex());
                CCLOG("select child end index = %d", item->getTag());

                int id = item->getTag();
                Button *btn = dynamic_cast<Button*>(item);
                log("+++");

                if (btn->getName() == "Node") {
                    auto queryData = [this, id, hideLoading]() {
                        auto node = TutorialData::getInstance()->getTutorialNode(id);
                        if (node.type == 0) {
                            //XQF
                            auto xqFile = new XQJsonFile();
                            xqFile->load(node.content);
                            getScheduler()->performFunctionInCocosThread(
                                    [this, xqFile, hideLoading](){
                                    auto scene = TutorialScene::create(xqFile);
                                    Director::getInstance()->pushScene(scene);
                                    hideLoading();
                                    this->release();
                                });
                        } else if (node.type == 1) {
                            //TXT
                            log("%s", node.content.c_str());
                            getScheduler()->performFunctionInCocosThread(
                                [this, hideLoading](){
                                    hideLoading();
                                    this->release();
                                });
                        }
                    };

                    this->retain();
                    auto thread = std::thread([queryData]{ queryData(); });
                    thread.detach();

                    for (auto &item : _listview->getItems())
                        item->removeFromParent();

                    showLoading();

                } else if (btn->getName() == "Category") {
					auto scene = TutorialMenuScene::create();
					scene->load(id);
                    Director::getInstance()->pushScene(scene);
                }
                log("---");

                break;
            }
        default:
            break;
    }
    });

    auto vbox = VBox::create();
    _headview = Layout::create();

    vbox->setContentSize(vsize);
    _headview->setContentSize(Size(vsize.width, 200));
    _listview->setContentSize(Size(vsize.width, vsize.height-200-100));

    _headview->setLayoutParameter(lp->clone());
    _listview->setLayoutParameter(lp->clone());

    vbox->addChild(_headview);
    vbox->addChild(_listview);

    vbox->setBackGroundImage("MainMenuScene/common_bg_new2.png");
    addChild(vbox);
    _layout = vbox;

    auto back = Button::create("DifficultyScene/look_back.png");
    back->addClickEventListener([](Ref *ref){
            Director::getInstance()->popScene();
            });
    back->setZoomScale(0.1);
    back->setPosition(Vec2(100, vsize.height-100));
    addChild(back);

	_pid = 1;

	setOnEnterCallback([this, refresh](){
            refresh(_pid);
	});

    return true;
}

TutorialMenuScene::~TutorialMenuScene()
{
    CC_SAFE_RELEASE(_category_button);
    CC_SAFE_RELEASE(_node_button);
}
