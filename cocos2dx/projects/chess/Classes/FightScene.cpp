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

bool FightScene::init(Role white, Role black, int level, std::string fen)
{
	if (!Scene::init())
		return false;

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

	auto lheader = HeaderSprite::createWithType(HeaderSprite::Type::LEFT);
	auto rheader = HeaderSprite::createWithType(HeaderSprite::Type::RIGHT);
    lheader->addClickEventListener([](Ref *ref) {
            int win = UserData::getInstance()->getIntegerForKey("FightScene:WIN", 0);
            int draw = UserData::getInstance()->getIntegerForKey("FightScene:DRAW", 0);
            int lose = UserData::getInstance()->getIntegerForKey("FightScene:LOSE", 0);
            PopupMessage::create(TR("The number of win:") + Utils::toString(win) + "\n" +
                                 TR("The number of lost:") + Utils::toString(lose) + "\n" +
                                 TR("The number of draw:") + Utils::toString(draw));
        });
    rheader->addClickEventListener([this](Ref *ref) {
            PopupMessage::create(TR("level:") + Utils::toString(_level));
        });
	auto lsize = lheader->getContentSize();
	auto rsize = rheader->getContentSize();
	lheader->setPosition(origin.x+vsize.width/2, origin.y+lsize.height/2+20);
	rheader->setPosition(origin.x+vsize.width/2, origin.y+vsize.height-rsize.height/2-20);

	/* GameLayer will add player & board as childs */
	_gameLayer = GameLayer::create(_playerWhite, _playerBlack, _board);
	_gameLayer->addChild(lheader);
	_gameLayer->addChild(rheader);
	addChild(_gameLayer);

	auto fightControl = FightControlLayer::create();
	addChild(fightControl);

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
	auto white_start_cb = [this, lheader, rheader, rotation, fightControl](EventCustom* ev){
		lheader->setActive(!rotation);
		rheader->setActive(rotation);
        /*
        int val1 = Rule::getInstance()->getWhiteLife(_board->getFenWithMove());
        int val2 = Rule::getInstance()->getBlackLife(_board->getFenWithMove());
        lheader->setInfoLine(Utils::toString(rotation ? val2 : val1));
        rheader->setInfoLine(Utils::toString(rotation ? val1 : val2));
        */
        fightControl->setEnabled(!rotation);
	};
	auto black_start_cb = [this, lheader, rheader, rotation, fightControl](EventCustom* ev){
		rheader->setActive(!rotation);
		lheader->setActive(rotation);
        /*
        int val1 = Rule::getInstance()->getWhiteLife(_board->getFenWithMove());
        int val2 = Rule::getInstance()->getBlackLife(_board->getFenWithMove());
        lheader->setInfoLine(Utils::toString(rotation ? val2 : val1));
        rheader->setInfoLine(Utils::toString(rotation ? val1 : val2));
        */
        fightControl->setEnabled(rotation);
	};

	/* response for reset */
	auto reset_cb = [this](EventCustom* ev) {
		removeAllChildren();
		auto scene = FightScene::create(_roleWhite, _roleBlack, _level, _fen);
		Director::getInstance()->replaceScene(scene);
	};
	/* response for switch */
	auto switch_cb = [this](EventCustom* ev) {
		removeAllChildren();
		auto scene = FightScene::create(_roleBlack, _roleWhite, _level, _fen);
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
        Sprite *sprite;

        if (event.find("DRAW:") != std::string::npos) {
            Sound::getInstance()->playEffect("draw");
            int count = UserData::getInstance()->getIntegerForKey("FightScene:DRAW", 0);
            UserData::getInstance()->setIntegerForKey("FightScene:DRAW", count+1);
            sprite = Sprite::create("ChallengeScene/text_hq.png");
        } else if (((event.find("WIN:WHITE") != std::string::npos) && (_roleWhite == Role::UI))
                   || ((event.find("WIN:BLACK") != std::string::npos) && (_roleBlack == Role::UI))) {
            Sound::getInstance()->playEffect("win");
            int count = UserData::getInstance()->getIntegerForKey("FightScene:WIN", 0);
            UserData::getInstance()->setIntegerForKey("FightScene:WIN", count+1);
            sprite = Sprite::create("ChallengeScene/text_yq.png");
        } else {
            Sound::getInstance()->playEffect("lose");
            int count = UserData::getInstance()->getIntegerForKey("FightScene:LOSE", 0);
            UserData::getInstance()->setIntegerForKey("FightScene:LOSE", count+1);
            sprite = Sprite::create("ChallengeScene/text_sq.png");
        }
        addChild(sprite);
        sprite->setPosition(origin.x + vsize.width/2, origin.y + vsize.height/2);

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

	setOnEnterCallback([this, reset_cb, switch_cb, save_cb, tip_cb, white_start_cb, black_start_cb, over_cb](){
		getEventDispatcher()->addCustomEventListener(EVENT_RESET, reset_cb);
		getEventDispatcher()->addCustomEventListener(EVENT_SWITCH, switch_cb);
		getEventDispatcher()->addCustomEventListener(EVENT_SAVE, save_cb);
		getEventDispatcher()->addCustomEventListener(EVENT_TIP, tip_cb);
		getEventDispatcher()->addCustomEventListener(EVENT_WHITE_START, white_start_cb);
		getEventDispatcher()->addCustomEventListener(EVENT_BLACK_START, black_start_cb);
        getEventDispatcher()->addCustomEventListener(EVENT_GAMEOVER, over_cb);
	});

	setOnExitCallback([this](){
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
