#include "FightScene.h"
#include "UserData.h"
#include "GameLayer.h"
#include "BGLayer.h"
#include "FightControl.h"
#include "HeaderSprite.h"
#include "Sound.h"

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
	auto lsize = lheader->getContentSize();
	auto rsize = rheader->getContentSize();
	lheader->setPosition(origin.x+vsize.width/2, origin.y+lsize.height/2+20);
	rheader->setPosition(origin.x+vsize.width/2, origin.y+vsize.height-rsize.height/2-20);

	/* GameLayer will add player & board as childs */
	_gameLayer = GameLayer::create(_playerWhite, _playerBlack, _board);
	_gameLayer->addChild(lheader);
	_gameLayer->addChild(rheader);
	addChild(_gameLayer);

	auto fightControl = FightControl::create();
	addChild(fightControl);

	_roleWhite = white;
	_roleBlack = black;
	_level = level;
	_fen = fen;

	/* rotation for UI black player */
	bool rotation = false;
	if (black == Role::UI) {
		_board->setRotation(180);
		rotation = true;
	}

	/* active/deactive header */
	auto white_start_cb = [this, lheader, rheader, rotation](EventCustom* ev){
		lheader->setActive(!rotation);
		rheader->setActive(rotation);
	};
	auto black_start_cb = [this, lheader, rheader, rotation](EventCustom* ev){
		rheader->setActive(!rotation);
		lheader->setActive(rotation);
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
		se.roleWhite = _roleWhite;
		se.roleBlack = _roleBlack;
		se.level = _level;
		se.white = "white";
		se.black = "black";
		se.fen = _board->getFenWithMove();
		UserData::getInstance()->insertSaveElement(se);
	};
    auto over_cb = [this](EventCustom *ev) {
        std::string event = (const char *)ev->getUserData();
        if (event.find("DRAW:") != std::string::npos) {
            Sound::getInstance()->playEffect("draw");
        } else if (((event.find("WIN:WHITE") != std::string::npos) && (_roleWhite == Role::UI))
                   || ((event.find("WIN:BLACK") != std::string::npos) && (_roleBlack == Role::UI))) {
            Sound::getInstance()->playEffect("win");
        } else {
            Sound::getInstance()->playEffect("lose");
        }
    };

	setOnEnterCallback([this, reset_cb, switch_cb, save_cb, white_start_cb, black_start_cb, over_cb](){
		getEventDispatcher()->addCustomEventListener(EVENT_RESET, reset_cb);
		getEventDispatcher()->addCustomEventListener(EVENT_SWITCH, switch_cb);
		getEventDispatcher()->addCustomEventListener(EVENT_SAVE, save_cb);
		getEventDispatcher()->addCustomEventListener(EVENT_WHITE_START, white_start_cb);
		getEventDispatcher()->addCustomEventListener(EVENT_BLACK_START, black_start_cb);
        getEventDispatcher()->addCustomEventListener(EVENT_GAMEOVER, over_cb);
	});

	setOnExitCallback([this](){
		getEventDispatcher()->removeCustomEventListeners(EVENT_RESET);
		getEventDispatcher()->removeCustomEventListeners(EVENT_SWITCH);
		getEventDispatcher()->removeCustomEventListeners(EVENT_SAVE);
		getEventDispatcher()->removeCustomEventListeners(EVENT_WHITE_START);
		getEventDispatcher()->removeCustomEventListeners(EVENT_BLACK_START);
		getEventDispatcher()->removeCustomEventListeners(EVENT_GAMEOVER);
	});

	return true;
}
