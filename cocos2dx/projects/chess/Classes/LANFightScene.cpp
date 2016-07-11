#include "LANFightScene.h"
#include "BGLayer.h"
#include "HeaderSprite.h"
#include "UIPlayer.h"
#include "NetPlayer.h"

bool LANFightScene::init()
{
	if (!Scene::init())
		return false;

	_client = nullptr;
	_server = nullptr;

	auto bgLayer = BGLayer::create();
	addChild(bgLayer);

	_fightControl = FightControl::create();
	addChild(_fightControl, 1);

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
		addChild(_gameLayer, 0, "gamelayer");
		_playerWhite->release();
		_playerBlack->release();
		if (ev->getUserData() == nullptr) {
			_playerNet->onRequest("reset");
		}
        removeChild(_fightControl);
        _fightControl = FightControl::create();
        addChild(_fightControl, 1);
	};
	/* response for switch */
	auto switch_cb = [this](EventCustom* ev) {
		_playerWhite->retain();
		_playerBlack->retain();
		removeChild(_gameLayer);
		_board = Board::create();
        _side = _side == "white" ? "black" : "white";
		std::swap(_playerWhite, _playerBlack);
		if (_playerNet != _playerBlack)
			_board->setRotation(180);
		_playerUI->setBoard(_board);
		_gameLayer = GameLayer::create(_playerWhite, _playerBlack, _board);
		addChild(_gameLayer, 0, "gamelayer");
		_playerWhite->release();
		_playerBlack->release();
		if (ev->getUserData() == nullptr) {
			_playerNet->onRequest("switch");
		}
        removeChild(_fightControl);
        _fightControl = FightControl::create();
        addChild(_fightControl, 1);
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
			log("showLoading");
            _fightControl->setEnabled(false);
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
            log("startGame %s %s", _side.c_str(), _fen.c_str());

            if (getChildByName("gamelayer")) {
                removeChildByName("gamelayer");
            }

            _fightControl->setEnabled(true);

			_board = Board::createWithFen(_fen);
			_playerUI = UIPlayer::create();
			_playerUI->setBoard(_board);
			_playerWhite = _playerUI;
			_playerBlack =_playerNet = NetPlayer::create(_client);
			if (_side == "black") {
				std::swap(_playerWhite, _playerBlack);
				_board->setRotation(180);
			}
			_gameLayer = GameLayer::create(_playerWhite, _playerBlack, _board);
			addChild(_gameLayer, 0, "gamelayer");

			removeChildByName("outter");
			removeChildByName("inner");
		});
	};

    auto onServer = [=]() {
        log("onServer");
        /* we may not recv the connect event */
        _client->on("connect", [this](RoomClient *client, const RoomPacket &packet) {
                log("server: client connect");
                RoomPacket p;
                p["TYPE"] = "init";
                p["TO"] = "all";
                p["SIDE"] = _side;
                p["FEN"] = _fen;
                 _client->emitPacket(p);
            });

        /* client may join before us */
        if (_client->getMemberList().size() != 0) {
            RoomPacket p;
            p["TYPE"] = "init";
            p["TO"] = "all";
            p["SIDE"] = _side;
            p["FEN"] = _fen;
            _client->emitPacket(p);
        }
        _client->on("init", [this, startGame](RoomClient *client, const RoomPacket &packet) {
                log("server: onInit");
                startGame();
            });
        /* server: client disconnect */
        _client->on("disconnect", [this, showLoading](RoomClient *client, const RoomPacket &packet) {
                log("server: client disconnect");
                getScheduler()->performFunctionInCocosThread([this](){
                        _playerWhite->stop();
                        _playerBlack->stop();
                        _fen = _board->getFenWithMove();
                     });
                showLoading();
            });
    };

    auto onClient = [=]() {
        _client->on("init", [this, startGame](RoomClient *client, const RoomPacket &packet) {
                log("client: onInit (%s %s)", packet["SIDE"].c_str(), packet["FEN"].c_str());

                _client->emit("init", "ok");

                _side = packet["SIDE"];
                _side = _side == "white" ? "black" : "white";
                _fen = packet["FEN"];
                startGame();
            });
        /* client: server shutdown */
        _client->on("disconnect", [this, showLoading, onServer] (RoomClient *client, const RoomPacket &packet) {

                log("client: server shutdown");
                showLoading();
                /* switch to server */
                RoomManager *manager = RoomManager::getInstance();
                _server = manager->createRoom();
                std::vector<RoomManager::RoomInfo> v;
                v = manager->scanRoom(1, 5);
                if (v.size() == 0)
                    return;
                auto clientOld = _client;
                _client = manager->joinRoom(_clientID, v[0]);
                getScheduler()->performFunctionInCocosThread([this, clientOld, onServer](){
                        _playerWhite->stop();
                        _playerBlack->stop();
                        /* ensure stop players before */
                        RoomManager::getInstance()->leaveRoom(clientOld);
                        _fen = _board->getFenWithMove();
                        onServer();
                    });
            });
    };

	showLoading();

	RoomManager *manager = RoomManager::getInstance();
	std::vector<RoomManager::RoomInfo> v;
	v = manager->scanRoom(1, 3);
	if (v.size() == 0) {
		_server = manager->createRoom();
		v = manager->scanRoom(1, 5);
	}
	if (v.size() == 0) {
		log("maybe no connection");
		getScheduler()->performFunctionInCocosThread([](){
			Director::getInstance()->popToRootScene();
		});
		return;
	}

	char name[16];
	std::srand(std::time(0));
	snprintf(name, sizeof(name), "client-%d", int(std::rand()*1000.0f/RAND_MAX));
	_client = manager->joinRoom(name, v[0]);
	log("clientID: %s", name);
    _clientID = name;

    if (_server) {
        _side = "white";
        _fen = Board::START_FEN;
        onServer();
    } else {
        onClient();
    }
}
