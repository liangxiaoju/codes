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
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([this](){
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

	RoomManager *manager = RoomManager::getInstance();
	std::vector<RoomManager::RoomInfo> v;
	v = manager->scanRoom(1, 2);
	if (v.size() == 0) {
		_server = manager->createRoom();
		v = manager->scanRoom(1, 5);
	}
	if (v.size() == 0) {
		Director::getInstance()->getScheduler()->performFunctionInCocosThread([](){
			Director::getInstance()->popScene();
		});
		return;
	}

	char name[16];
	std::srand(std::time(0));
	snprintf(name, sizeof(name), "%d", int(std::rand()*1000.0f/RAND_MAX));
	_client = manager->joinRoom(name, v[0]);

	Director::getInstance()->getScheduler()->performFunctionInCocosThread([this](){
		_board = Board::create();
		auto player = UIPlayer::create();
		player->setBoard(_board);
		_playerWhite = player;
		_playerNet = _playerBlack = NetPlayer::create(_client);
		if (!_server) {
			std::swap(_playerWhite, _playerBlack);
			_board->setRotation(180);
		}
		_gameLayer = GameLayer::create(_playerWhite, _playerBlack, _board);
		addChild(_gameLayer);

		removeChildByName("outter");
		removeChildByName("inner");
	});
}
