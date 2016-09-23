#include "LANFightScene.h"
#include "BGLayer.h"
#include "HeaderSprite.h"
#include "UIPlayer.h"
#include "NetPlayer.h"
#include "Sound.h"
#include "Localization.h"
#include "CheckBoxItem.h"
#include "PopupBox.h"

bool LANScene::init()
{
    if (!Scene::init())
        return false;

	_isDestroyed = std::make_shared<std::atomic<bool>>(false);

    auto vsize = Director::getInstance()->getVisibleSize();

    _layout = RelativeBox::create();
    _layout->setContentSize(vsize);
    _layout->setBackGroundImage("MainMenuScene/common_bg_new2.png");
    addChild(_layout);

    RelativeLayoutParameter *param;
    param = RelativeLayoutParameter::create();
    param->setAlign(RelativeLayoutParameter::RelativeAlign::PARENT_TOP_CENTER_HORIZONTAL);
    param->setMargin(Margin(0, 50, 0, 50));
    param->setRelativeName("title");

    _text = Text::create("", "", 50);
    _text->setLayoutParameter(param);
    _layout->addChild(_text);

    param = RelativeLayoutParameter::create();
    //param->setRelativeToWidgetName("title");
    param->setAlign(RelativeLayoutParameter::RelativeAlign::CENTER_IN_PARENT);

    _viewLayout = VBox::create();
    _viewLayout->setBackGroundImage("pop_bg.png");
    _viewLayout->setBackGroundImageScale9Enabled(true);
    _viewLayout->setBackGroundImageCapInsets(Rect(15, 15, 565-30, 199-30));
    _viewLayout->setLayoutParameter(param);
    _viewLayout->setContentSize(Size(100, 50));
    _layout->addChild(_viewLayout);

    param = RelativeLayoutParameter::create();
    param->setAlign(RelativeLayoutParameter::RelativeAlign::PARENT_TOP_LEFT);
    param->setMargin(Margin(36, 41, 0, 0));

    auto exit = Button::create("FightSceneMenu/look_back.png");
    exit->setZoomScale(0.1);
    exit->addClickEventListener([this](Ref *ref) {
            Director::getInstance()->popScene();
        });
    exit->setLayoutParameter(param);
    _layout->addChild(exit);

    return true;
}

LANScene::~LANScene()
{
	*_isDestroyed = true;
}

void LANScene::setTitle(const std::string &title)
{
    _text->setString(title);
}

void LANScene::pushBackView(Widget *child)
{
    auto param = LinearLayoutParameter::create();
    param->setGravity(LinearLayoutParameter::LinearGravity::CENTER_HORIZONTAL);
    param->setMargin(Margin(0, 50, 0, 0));

    child->setLayoutParameter(param);
    _viewLayout->addChild(child);

    auto vsize = Director::getInstance()->getVisibleSize();
    auto wsize = child->getContentSize();
    auto lsize = _viewLayout->getContentSize();
    float width = std::max(lsize.width, wsize.width+100);
    float height = std::min(lsize.height+50+wsize.height, vsize.height);
    _viewLayout->setContentSize(Size(width, height));
}

bool LANTopScene::init()
{
    if (!LANScene::init())
        return false;

    setTitle(TR("LAN"));

    auto create = Button::create("button.png");
    create->setZoomScale(0.1);
    create->setTitleFontSize(35);
    create->setTitleFontName("");
    create->setTitleText(TR("create"));
    create->addClickEventListener([this](Ref *ref) {
            auto scene = LANCreateScene::create();
            Director::getInstance()->pushScene(scene);
        });

    auto join = Button::create("button.png");
    join->setZoomScale(0.1);
    join->setTitleFontSize(35);
    join->setTitleFontName("");
    join->setTitleText(TR("join"));
    join->addClickEventListener([this](Ref *ref) {
            auto scene = LANJoinScene::create();
            Director::getInstance()->pushScene(scene);
        });

    pushBackView(create);
    pushBackView(join);

    return true;
}

bool LANCreateScene::init()
{
    if (!LANScene::init())
        return false;

    setTitle(TR("Create"));

    auto load = Button::create("button.png");
    load->setZoomScale(0.1);
    load->setTitleFontSize(35);
    load->setTitleFontName("");
    load->setTitleText(TR("load"));
    load->addClickEventListener([this](Ref *ref) {
            auto scene = LANLoadScene::create();
            Director::getInstance()->replaceScene(scene);
        });

    auto create = Button::create("button.png");
    create->setZoomScale(0.1);
    create->setTitleFontSize(35);
    create->setTitleFontName("");
    create->setTitleText(TR("create"));
    create->addClickEventListener([this](Ref *ref) {
            auto scene = LANWaitScene::create(_fen, _side);
            Director::getInstance()->replaceScene(scene);
        });

    _fen = Board::START_FEN;
    _side = "white";
    auto side_select_cb = [this](bool selected) {
        _side = selected ? "black" : "white";
    };

    auto allow_regret_cb = [this](bool selected) {
        _regret = selected;
    };

    pushBackView(CheckBoxItem::create(TR("Play as Black:"), side_select_cb, false));
    pushBackView(CheckBoxItem::create(TR("Allow to Regret:"), allow_regret_cb, false));
    pushBackView(load);
    pushBackView(create);

    return true;
}

bool LANLoadScene::init()
{
    if (!LANScene::init())
        return false;

    auto vsize = Director::getInstance()->getVisibleSize();

    auto listview = ListView::create();
    listview->setDirection(ScrollView::Direction::VERTICAL);
    listview->setBounceEnabled(true);
    listview->setScrollBarEnabled(false);
    listview->setGravity(ListView::Gravity::CENTER_HORIZONTAL);
    listview->setItemsMargin(50);
    listview->addEventListener([this](Ref *pSender, ListView::EventType type) {
            switch (type) {
            case ListView::EventType::ON_SELECTED_ITEM_END:
            {
                ListView* listView = static_cast<ListView*>(pSender);
                auto item = listView->getItem(listView->getCurSelectedIndex());
                Button *btn = dynamic_cast<Button*>(item);
                int index = btn->getTag();

                for (auto &se : _saveElements) {
                    if (se.id == index) {
                        std::string fen = se.content;
                        std::string side = se.roleWhite ? "white" : "black";
                        auto scene = LANWaitScene::create(fen, side);
                        Director::getInstance()->replaceScene(scene);
                    }
                }
            }
            default:
                break;
            }
        });
    listview->setContentSize(Size(vsize.width*2/3, vsize.height/2));

    pushBackView(listview);

    setOnEnterCallback([this, listview]() {
            listview->removeAllItems();

            UserData::getInstance()->querySaveTbl(_saveElements);

            for (size_t i = 0; i < _saveElements.size(); i++) {
                UserData::SaveElement se = _saveElements[i];
                if (se.type == TYPE_LANFIGHT) {
                    auto btn = Button::create("button.png");
                    btn->setTitleFontSize(35);
                    btn->setTitleText(se.date);
                    btn->setTag(se.id);
                    listview->pushBackCustomItem(btn);
                }
            }
        });

    return true;
}

bool LANWaitScene::init(std::string fen, std::string side)
{
    if (!LANScene::init())
        return false;

    setTitle(TR("Wait"));

    _fen = fen;
    _side = side;
    _server = nullptr;
    _client = nullptr;

    auto vsize = Director::getInstance()->getVisibleSize();

    auto text = Text::create("Side: " + side + "\n\nWaiting ...", "", 35);
    text->setTextColor(Color4B::GRAY);

    pushBackView(text);

    std::shared_ptr<std::atomic<bool>> isDestroyed = _isDestroyed;

    auto createDone = [this, isDestroyed](void *param) {
        if (*isDestroyed) {
            log("LANWaitScene instance was destroyed!");
            return;
        }
        onWait();
    };

    AsyncTaskPool::getInstance()->enqueue(
        AsyncTaskPool::TaskType::TASK_NETWORK, createDone, nullptr,
        [this, isDestroyed](){
            if (*isDestroyed) {
                log("LANWaitScene instance was destroyed!");
                return;
            }
            _server = RoomManager::getInstance()->createRoom("", "");

            char name[16];
            std::srand(std::time(0));
            snprintf(name, sizeof(name), "server-%d",
                     int(std::rand()*1000.0f/RAND_MAX));

            auto infos = RoomManager::getInstance()->scanRoom(1, 5);
            if (infos.size() == 0) {
                log("cannot find our created room !");
                infos = RoomManager::getInstance()->scanRoom(1, 10);
                if (infos.size() == 0)
                    return;
            }
            _client = RoomManager::getInstance()->joinRoom(name, infos[0]);
        });

    return true;
}

LANWaitScene::~LANWaitScene()
{
    if (_client)
        RoomManager::getInstance()->leaveRoom(_client);
    if (_server)
        RoomManager::getInstance()->destroyRoom(_server);
}

void LANWaitScene::onWait()
{
    log("onWait");

    if (_client == nullptr) {
        log("client is null");
        return;
    }

    std::shared_ptr<std::atomic<bool>> isDestroyed = _isDestroyed;

    /* we may not recv the connect event */
    _client->on("connect", [this, isDestroyed](RoomClient *client, const RoomPacket &packet) {
            log("server: client connect");

 			if (*isDestroyed) {
				log("LANWaitScene instance was destroyed!");
				return;
			}

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

    _client->on("init", [this, isDestroyed](RoomClient *client, const RoomPacket &packet) {
            log("server: onInit");

 			if (*isDestroyed) {
				log("LANWaitScene instance was destroyed!");
				return;
			}

            Director::getInstance()->getScheduler()
                ->performFunctionInCocosThread([this] {
                        auto scene = LANFightScene::create(
                            _fen, _side, _server, _client);
                        /* let LANFightScene delete _server && _client */
                        _server = nullptr;
                        _client = nullptr;
                        Director::getInstance()->replaceScene(scene);
                    });
        });
}

bool LANJoinScene::init()
{
    if (!LANScene::init())
        return false;

    _client = nullptr;

    auto vsize = Director::getInstance()->getVisibleSize();

    setTitle(TR("Join"));

    auto listview = ListView::create();
    listview->setDirection(ScrollView::Direction::VERTICAL);
    listview->setBounceEnabled(true);
    listview->setScrollBarEnabled(false);
    listview->setGravity(ListView::Gravity::CENTER_HORIZONTAL);
    listview->setItemsMargin(50);
    listview->addEventListener([this](Ref *pSender, ListView::EventType type) {
            switch (type) {
            case ListView::EventType::ON_SELECTED_ITEM_START:
            {
                ListView* listView = static_cast<ListView*>(pSender);
                break;
            }
            case ListView::EventType::ON_SELECTED_ITEM_END:
            {
                ListView* listView = static_cast<ListView*>(pSender);
                auto item = listView->getItem(listView->getCurSelectedIndex());
                Button *btn = dynamic_cast<Button*>(item);
                int index = btn->getTag();
                log("search list: %d clicked", index);
                if (index < _roomInfos.size()) {
                    auto info = _roomInfos[index];
                    char name[16];
                    std::srand(std::time(0));
                    snprintf(name, sizeof(name), "client-%d", int(std::rand()*1000.0f/RAND_MAX));
                    _client = RoomManager::getInstance()->joinRoom(name, info);
                    onJoin();
                }
                break;
            }
            default:
                break;
            }
        });
    listview->setContentSize(Size(vsize.width*2/3, vsize.height/2));

    auto do_search = [this, listview](Ref *ref) {
        std::shared_ptr<std::atomic<bool>> isDestroyed = _isDestroyed;

        auto search = dynamic_cast<Button*>(ref);

        listview->removeAllItems();
        search->setEnabled(false);

        auto scanDone = [listview, search, this, isDestroyed](void *param) {
 			if (*isDestroyed) {
				log("LANJoinScene instance was destroyed!");
				return;
			}

            log("search: done %d", _roomInfos.size());
            for (int i = 0; i < _roomInfos.size(); i++) {
                auto info = _roomInfos[i];
                auto btn = Button::create("button.png");
                btn->setTag(i);
                btn->setTitleFontSize(20);
                btn->setTitleText(info.host);
                listview->pushBackCustomItem(btn);
                log("%s", info.host.c_str());
            }
            search->setEnabled(true);
        };

        AsyncTaskPool::getInstance()->enqueue(
            AsyncTaskPool::TaskType::TASK_NETWORK, scanDone, nullptr, [this, isDestroyed](){
                if (*isDestroyed) {
                    log("LANJoinScene instance was destroyed!");
                    return;
                }
                auto manager = RoomManager::getInstance();
                auto infos = manager->scanRoom(5, 5);
                if (*isDestroyed) {
                    log("LANJoinScene instance was destroyed!");
                    return;
                }
                _roomInfos = infos;
            });
    };

    auto search = Button::create("button.png");
    search->setZoomScale(0.1);
    search->setTitleFontSize(35);
    search->setTitleFontName("");
    search->setTitleText(TR("search"));
    search->addClickEventListener(do_search);

    pushBackView(listview);
    pushBackView(search);

    setOnEnterCallback([do_search, search]() {
            do_search(search);
        });

    return true;
}

void LANJoinScene::onJoin()
{
    if (_client == nullptr) {
        log("client is null");
        return;
    }

    std::shared_ptr<std::atomic<bool>> isDestroyed = _isDestroyed;

    _client->on("init", [this, isDestroyed](RoomClient *client, const RoomPacket &packet) {
            log("client: onInit");

 			if (*isDestroyed) {
				log("LANJoinScene instance was destroyed!");
				return;
			}

            /* tell server side to start game */
            _client->emit("init", "ok");

            _side = packet["SIDE"] == "white" ? "black" : "white";
            _fen = packet["FEN"];
            Director::getInstance()->getScheduler()
                ->performFunctionInCocosThread([this] {
                        auto scene = LANFightScene::create(_fen, _side, nullptr, _client);
                        Director::getInstance()->replaceScene(scene);
                });
        });
}

bool LANFightScene::init(std::string fen, std::string side,
                         RoomServer *server, RoomClient *client)
{
    if (!Scene::init())
        return false;

	_isDestroyed = std::make_shared<std::atomic<bool>>(false);

    auto vsize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    _fen = fen;
    _side = side;
    _server = server;
    _client = client;

    auto bgLayer = BGLayer::create();
    addChild(bgLayer);

    _board = Board::createWithFen(_fen);
    _playerUI = UIPlayer::create();
    _playerUI->setBoard(_board);
    _playerWhite = _playerUI;
    _playerBlack = _playerNet = NetPlayer::create(_client);
    if (_side == "black") {
        std::swap(_playerWhite, _playerBlack);
        _board->setRotation(180);
    }

    auto gameLayer = GameLayer::create(_playerWhite, _playerBlack, _board);
    addChild(gameLayer);

	auto lheader = HeaderSprite::createWithType(HeaderSprite::Type::LEFT);
	auto rheader = HeaderSprite::createWithType(HeaderSprite::Type::RIGHT);
	auto lsize = lheader->getContentSize();
	auto rsize = rheader->getContentSize();
	lheader->setPosition(origin.x+vsize.width/2, origin.y+lsize.height/2+20);
	rheader->setPosition(origin.x+vsize.width/2, origin.y+vsize.height-rsize.height/2-20);
	gameLayer->addChild(lheader);
	gameLayer->addChild(rheader);

    auto control = LANFightControlLayer::create();
    addChild(control, 1);

	/* active/deactive header */
	auto white_start_cb = [this, lheader, rheader, control](EventCustom* ev){
		lheader->setActive(_side == "white");
		rheader->setActive(_side == "black");
        control->setEnabled(_side == "white");
	};
	auto black_start_cb = [this, lheader, rheader, control](EventCustom* ev){
		rheader->setActive(_side == "white");
		lheader->setActive(_side == "black");
        control->setEnabled(_side == "black");
	};

    auto reset_cb = [this](EventCustom *ev) {

        std::string request;

        if (ev->getUserData() == nullptr)
            request = "UI";
        else
            request = (const char *)ev->getUserData();

        auto reset_scene = [this]() {
            auto scene = LANFightScene::create(_fen, _side, _server, _client);

            _server = nullptr;
            _client = nullptr;

            Director::getInstance()->replaceScene(scene);
        };

        if (request.find("Net") != std::string::npos) {
            log("reset request from Net");
            auto callback = [this, reset_scene](bool reply) {
                if (reply) {
                    reset_scene();
                    _playerNet->onReply("reset", "accept");
                } else {
                    _playerNet->onReply("reset", "deny");
                }
            };

            DialogBox::create(TR("Allow to reset ?"), TR("Yes"), TR("No"), callback);
        } else {
            log("reset request from UI");
            auto callback = [this, reset_scene](bool reply) {
                if (reply) {
                    reset_scene();
                } else {
                    PopupMessage::create(TR("Deny to reset"));
                }
            };

            _playerNet->onRequest("reset", "", callback);
        }
    };

    auto save_cb = [this](EventCustom *ev) {
        UserData::SaveElement se;
        se.type = TYPE_LANFIGHT;
        se.roleWhite = _side == "white" ? 1 : 0;
        se.roleBlack = _side == "black" ? 1 : 0;
        se.level = 0;
        se.white = "white";
        se.black = "black";
        se.content = _board->getFenWithMove();
        UserData::getInstance()->insertSaveElement(se);

        PopupMessage::create(TR("Save Done"));
    };

    auto over_cb = [this, origin, vsize](EventCustom *ev) {
        std::string event = (const char *)ev->getUserData();
        Sprite *sprite;

        if (event.find("DRAW:") != std::string::npos) {
            int count = UserData::getInstance()->getIntegerForKey(
                "LANFightScene:DRAW", 0);
            UserData::getInstance()->setIntegerForKey(
                "LANFightScene:DRAW", count+1);
            sprite = Sprite::create("board/watch_win_draw_tag.png");
        } else if (event.find("WIN:WHITE") != std::string::npos) {
            int count = UserData::getInstance()->getIntegerForKey(
                "LANFightScene:WIN:WHITE", 0);
            UserData::getInstance()->setIntegerForKey(
                "LANFightScene:WIN:WHITE", count+1);
            sprite = Sprite::create("board/watch_win_red_tag.png");
        } else {
            int count = UserData::getInstance()->getIntegerForKey(
                "LANFightScene:WIN:BLACK", 0);
            UserData::getInstance()->setIntegerForKey(
                "LANFightScene:WIN:BLACK", count+1);
            sprite = Sprite::create("board/watch_win_black_tag.png");
        }
        addChild(sprite);
        sprite->setPosition(origin.x + vsize.width/2, origin.y + vsize.height/2);
    };

    setOnEnterCallback([this, reset_cb, save_cb, over_cb, white_start_cb, black_start_cb]() {
            getEventDispatcher()->addCustomEventListener(EVENT_RESET, reset_cb);
            getEventDispatcher()->addCustomEventListener(EVENT_SAVE, save_cb);
            getEventDispatcher()->addCustomEventListener(EVENT_GAMEOVER, over_cb);
            getEventDispatcher()->addCustomEventListener(EVENT_WHITE_START, white_start_cb);
            getEventDispatcher()->addCustomEventListener(EVENT_BLACK_START, black_start_cb);
        });

    setOnExitCallback([this]() {
            getEventDispatcher()->removeCustomEventListeners(EVENT_RESET);
            getEventDispatcher()->removeCustomEventListeners(EVENT_SAVE);
            getEventDispatcher()->removeCustomEventListeners(EVENT_GAMEOVER);
            getEventDispatcher()->removeCustomEventListeners(EVENT_WHITE_START);
            getEventDispatcher()->removeCustomEventListeners(EVENT_BLACK_START);
        });

    std::shared_ptr<std::atomic<bool>> isDestroyed = _isDestroyed;

    _client->on("disconnect", [this, isDestroyed](RoomClient *client, const RoomPacket &packet) {
            log("room disconnect");

 			if (*isDestroyed) {
				log("LANFightScene instance was destroyed!");
				return;
			}

            Director::getInstance()->getScheduler()
                ->performFunctionInCocosThread([this] {
                        PopupMessage::create(TR("Lost connection"));
                    });
        });

    return true;
}

LANFightScene::~LANFightScene()
{
	*_isDestroyed = true;

    if (_client)
        RoomManager::getInstance()->leaveRoom(_client);
    if (_server)
        RoomManager::getInstance()->destroyRoom(_server);
}
