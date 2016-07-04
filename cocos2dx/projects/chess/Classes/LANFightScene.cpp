#include "LANFightScene.h"
#include "BGLayer.h"
#include "HeaderSprite.h"
#include "UIPlayer.h"
#include "NetPlayer.h"
#include "FightControl.h"

bool LANFightScene::init()
{
	if (!Scene::init())
		return false;

	_client = nullptr;
	_server = nullptr;

	auto bgLayer = BGLayer::create();
	addChild(bgLayer);

	auto fightControl = FightControl::create();
	addChild(fightControl, 1);

	/* response for reset */
	auto reset_cb = [this](EventCustom* ev) {
		_playerWhite->retain();
		_playerBlack->retain();
		removeChild(_gameLayer);
		_board = Board::create();
		if (_playerNet != _playerBlack)
			_board->setRotation(180);
		_playerUI->setBoard(_board);
		_gameLayer = GameLayer::create(_playerWhite, _playerBlack, _board);
		addChild(_gameLayer);
		_playerWhite->release();
		_playerBlack->release();
		if (ev->getUserData() == nullptr) {
			_playerNet->onRequest("reset");
		}
	};
	/* response for switch */
	auto switch_cb = [this](EventCustom* ev) {
		_playerWhite->retain();
		_playerBlack->retain();
		removeChild(_gameLayer);
		_board = Board::create();
		std::swap(_playerWhite, _playerBlack);
		if (_playerNet != _playerBlack)
			_board->setRotation(180);
		_playerUI->setBoard(_board);
		_gameLayer = GameLayer::create(_playerWhite, _playerBlack, _board);
		addChild(_gameLayer);
		_playerWhite->release();
		_playerBlack->release();
		if (ev->getUserData() == nullptr) {
			_playerNet->onRequest("switch");
		}
	};

	setOnEnterCallback([this, reset_cb, switch_cb](){
		_thread = std::thread(std::bind(&LANFightScene::scan, this));
		getEventDispatcher()->addCustomEventListener(EVENT_RESET, reset_cb);
		getEventDispatcher()->addCustomEventListener(EVENT_SWITCH, switch_cb);
	});

	setOnExitCallback([this](){
		getEventDispatcher()->removeCustomEventListeners(EVENT_RESET);
		getEventDispatcher()->removeCustomEventListeners(EVENT_SWITCH);
		_thread.join();
		if (_client)
			RoomManager::getInstance()->leaveRoom(_client);
		if (_server)
			RoomManager::getInstance()->destroyRoom(_server);
	});

	return true;
}

void LANFightScene::scan()
{
	auto showLoading = [this]() {
		getScheduler()->performFunctionInCocosThread([this](){
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
			});
	};

	auto startGame = [this]() {
		getScheduler()->performFunctionInCocosThread([this](){
			_board = Board::create();
			_playerUI = UIPlayer::create();
			_playerUI->setBoard(_board);
			_playerWhite = _playerUI;
			_playerBlack =_playerNet = NetPlayer::create(_client);
			if (!_server) {
				std::swap(_playerWhite, _playerBlack);
				_board->setRotation(180);
			}
			_gameLayer = GameLayer::create(_playerWhite, _playerBlack, _board);
			addChild(_gameLayer);

			removeChildByName("outter");
			removeChildByName("inner");
			});
	};

	showLoading();

	RoomManager *manager = RoomManager::getInstance();
	std::vector<RoomManager::RoomInfo> v;
	v = manager->scanRoom(1, 2);
	if (v.size() == 0) {
		_server = manager->createRoom();
		v = manager->scanRoom(1, 5);
	}
	if (v.size() == 0) {
		/* maybe no connection */
		getScheduler()->performFunctionInCocosThread([](){
			Director::getInstance()->popScene();
		});
		return;
	}

	char name[16];
	std::srand(std::time(0));
	snprintf(name, sizeof(name), "client-%d", int(std::rand()*1000.0f/RAND_MAX));
	_client = manager->joinRoom(name, v[0]);
	log("clientID: %s", name);

	_client->on("disconnect", [showLoading](RoomClient *client, const RoomPacket &packet) { showLoading(); });

	if (_server && (_client->getMemberList().size() == 0)) {
		_client->on("connect", [startGame](RoomClient *client, const RoomPacket &packet) { startGame(); });
	} else {
		startGame();
	}
}
