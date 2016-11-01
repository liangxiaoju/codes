#include "FightScene.h"
#include "UserData.h"
#include "GameLayer.h"
#include "BGLayer.h"
#include "XQFile/XQJsonFile.h"
#include "HeaderSprite.h"
#include "Sound.h"
#include "ControlLayer.h"
#include "PopupBox.h"
#include "Localization.h"
#include "SettingMenu.h"
#include "LevelMenu.h"
#include "GameOverView.h"
#include "HeaderView.h"

bool FightScene::init(Role white, Role black, int level, std::string fen)
{
	if (!Scene::init())
		return false;

    if (level < 0) {
        level = LevelMenu::getUserLevel();
    }

	auto vsize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	auto bgLayer = BGLayer::create();
	addChild(bgLayer);

	_board = Board::createWithFen(fen);

	auto createPlayer = [this, level](Role role)->Player*{
		if (role == Role::UI) {
			auto player = UIPlayer::create();
			player->setBoard(_board);
			return player;
		} else if (role == Role::AI) {
			auto player = AIPlayer::create();
			player->setLevel(level);
			return player;
		} else
			return nullptr;
	};

	_playerWhite = createPlayer(white);
	_playerBlack = createPlayer(black);

    auto head1 = EnemyHeaderView::create(1, Board::Side::BLACK);
    head1->setAnchorPoint(Vec2(0, 1));
    head1->setPosition(Vec2(origin.x, origin.y+vsize.height-20));
    addChild(head1);

    auto head2 = SelfHeaderView::create();
    head2->setAnchorPoint(Vec2(1, 0));
    head2->setPosition(Vec2(origin.x+vsize.width, origin.y+20));
    addChild(head2);

	/* GameLayer will add player & board as childs */
	_gameLayer = GameLayer::create(_playerWhite, _playerBlack, _board);
	addChild(_gameLayer);

    std::vector<std::string> menus = {
        TR("regret"), TR("tip"), TR("resign"), TR("again"), TR("level"), TR("setting")};
    std::vector<std::string> events = {
        EVENT_REGRET, EVENT_TIP, EVENT_RESIGN, EVENT_RESET, EVENT_LEVEL, EVENT_SETTING};

    auto menu = Button::create("common/menu.png");
    menu->addClickEventListener([this, menus, events](Ref *ref) {
            MenuBox::create(menus, [this, events](int index) {
                    getScheduler()->performFunctionInCocosThread([this, events, index]() {
                            if (events[index] == EVENT_SETTING) {
                                SettingMenu::getInstance()->show();
                            } else if (events[index] == EVENT_LEVEL) {
                                LevelMenu::create();
                            } else {
                                getEventDispatcher()->dispatchCustomEvent(events[index]);
                            }
                            });
                    });
        });
    menu->setTitleText(TR("Menu"));
    menu->setTitleFontSize(48);
    menu->setTitleColor(Color3B(231, 204, 143));
    menu->setAnchorPoint(Vec2::ZERO);
    menu->setPosition(Vec2(0, 20));
    addChild(menu);

	_roleWhite = white;
	_roleBlack = black;
	_level = level;
	_fen = fen;

	/* rotation for UI black player */
	bool rotation = false;
	if ((black == Role::UI) && (white != Role::UI)) {
		_board->setRotation(180);
		rotation = true;
	}

	/* active/deactive header */
	auto white_start_cb = [this, rotation, menu](EventCustom* ev){
		//lheader->setActive(!rotation);
		//rheader->setActive(rotation);
        /*
        int val1 = Rule::getInstance()->getWhiteLife(_board->getFenWithMove());
        int val2 = Rule::getInstance()->getBlackLife(_board->getFenWithMove());
        lheader->setInfoLine(Utils::toString(rotation ? val2 : val1));
        rheader->setInfoLine(Utils::toString(rotation ? val1 : val2));
        */
        menu->setEnabled(!rotation);
	};
	auto black_start_cb = [this, rotation, menu](EventCustom* ev){
		//rheader->setActive(!rotation);
		//lheader->setActive(rotation);
        /*
        int val1 = Rule::getInstance()->getWhiteLife(_board->getFenWithMove());
        int val2 = Rule::getInstance()->getBlackLife(_board->getFenWithMove());
        lheader->setInfoLine(Utils::toString(rotation ? val2 : val1));
        rheader->setInfoLine(Utils::toString(rotation ? val1 : val2));
        */
        menu->setEnabled(rotation);
	};

	/* response for reset */
	auto reset_cb = [this](EventCustom* ev) {
		removeAllChildren();
		auto scene = FightScene::create(_roleWhite, _roleBlack, -1, _fen);
		Director::getInstance()->replaceScene(scene);
	};
	/* response for switch */
	auto switch_cb = [this](EventCustom* ev) {
		removeAllChildren();
		auto scene = FightScene::create(_roleBlack, _roleWhite, -1, _fen);
		Director::getInstance()->replaceScene(scene);
	};
	/* response for save */
	auto save_cb = [this](EventCustom* ev) {
		UserData::SaveElement se;
        //se.type = TYPE_FIGHT;
		se.roleWhite = _roleWhite;
		se.roleBlack = _roleBlack;
		se.level = _level;
		se.white = "white";
		se.black = "black";
		se.content = _board->getFenWithMove();
		UserData::getInstance()->insertSaveElement(se);
	};
    /* response for tip */
    auto tip_cb = [this](EventCustom* ev) {
        Player *currentPlayer;
        if (_board->getCurrentSide() == Board::Side::WHITE) {
            currentPlayer = _playerWhite;
            /* no tip for AIPlayer */
            if (_roleWhite == Role::AI)
                return;
        } else {
            currentPlayer = _playerBlack;
            if (_roleBlack == Role::AI)
                return;
        }

        AIPlayer *ai;
        if (_roleWhite == Role::AI)
            ai = dynamic_cast<AIPlayer*>(_playerWhite);
        else if (_roleBlack == Role::AI)
            ai = dynamic_cast<AIPlayer*>(_playerBlack);
        else {
            return;
        }

        currentPlayer->stop();

        auto cb = [this, ai, currentPlayer](std::string mv) {
            log("TIP: %s", mv.c_str());
            currentPlayer->onRequest("move", mv);
        };
        ai->getHelp(_board->getFenWithMove(), cb);
    };

    auto over_cb = [this, origin, vsize](EventCustom *ev) {

        std::string event = (const char *)ev->getUserData();

        auto showOverView = [this, event]() {
            if (event.find("DRAW:") != std::string::npos) {
                FightDrawView::create();
            } else if (((event.find("WIN:WHITE") != std::string::npos)
                        && (_roleWhite == Role::UI))
                       || ((event.find("WIN:BLACK") != std::string::npos)
                           && (_roleBlack == Role::UI))) {
                FightWinView::create();
            } else {
                FightLoseView::create();
            }
        };

        auto filename = FileUtils::getInstance()->getWritablePath() + "/capture.png";
        Director::getInstance()->getTextureCache()->removeTextureForKey(filename);
        auto capture = utils::captureNode(_board, 0.7);
        capture->saveToFile(filename, false);
        showOverView();

        auto xqFile = new XQJsonFile();
        auto header = xqFile->getHeader();
        header->title = "FightScene";
        header->fen = _fen;
        for (auto &move : _board->getHistoryMoves()) {
            XQNode *node = new XQNode();
            node->mv = Utils::toUcciMove(move.src, move.dst);
            node->comment = move.comment;
            xqFile->addStep(node);
        }
        std::string json = xqFile->save();
        delete xqFile;
        log("FightScene: %s", json.c_str());

        UserData::RecordElement e;
        e.type = TYPE_FIGHT;
        e.roleWhite = _roleWhite;
        e.roleBlack = _roleBlack;
        e.level = _level;
        e.win = "";
        e.content = json;
        UserData::getInstance()->insertRecordElement(e);
    };

    auto level_cb = [this](EventCustom *ev) {
        int level = (int)ev->getUserData();
        _level = level;
        if (_roleWhite == Role::AI)
            dynamic_cast<AIPlayer*>(_playerWhite)->setLevel(level);
        else if (_roleBlack == Role::AI)
            dynamic_cast<AIPlayer*>(_playerBlack)->setLevel(level);
        log("set level to %d", level);
    };

	setOnEnterCallback(
        [this, level_cb, reset_cb, switch_cb, save_cb, tip_cb,
         white_start_cb, black_start_cb, over_cb](){
		getEventDispatcher()->addCustomEventListener(EVENT_LEVEL_CHANGE, level_cb);
		getEventDispatcher()->addCustomEventListener(EVENT_RESET, reset_cb);
		getEventDispatcher()->addCustomEventListener(EVENT_SWITCH, switch_cb);
		getEventDispatcher()->addCustomEventListener(EVENT_SAVE, save_cb);
		getEventDispatcher()->addCustomEventListener(EVENT_TIP, tip_cb);
		getEventDispatcher()->addCustomEventListener(EVENT_WHITE_START, white_start_cb);
		getEventDispatcher()->addCustomEventListener(EVENT_BLACK_START, black_start_cb);
        getEventDispatcher()->addCustomEventListener(EVENT_GAMEOVER, over_cb);
	});

	setOnExitCallback([this](){
		getEventDispatcher()->removeCustomEventListeners(EVENT_LEVEL_CHANGE);
		getEventDispatcher()->removeCustomEventListeners(EVENT_RESET);
		getEventDispatcher()->removeCustomEventListeners(EVENT_SWITCH);
		getEventDispatcher()->removeCustomEventListeners(EVENT_SAVE);
		getEventDispatcher()->removeCustomEventListeners(EVENT_TIP);
		getEventDispatcher()->removeCustomEventListeners(EVENT_WHITE_START);
		getEventDispatcher()->removeCustomEventListeners(EVENT_BLACK_START);
		getEventDispatcher()->removeCustomEventListeners(EVENT_GAMEOVER);
	});

	return true;
}
