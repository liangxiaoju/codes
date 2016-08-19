#include "LocalFightScene.h"

bool LocalFightScene::init()
{
    if (!Scene::init())
        return false;

    auto vsize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    auto bgLayer = BGLayer::create();
    addChild(bgLayer);

    _board = Board::create();
    _board->setRotation(90);

    _playerWhite = UIPlayer::create();
    _playerBlack = UIPlayer::create();
    _playerWhite->setBoard(_board);
    _playerBlack->setBoard(_board);

    _gameLayer = GameLayer::create(_playerWhite, _playerBlack, _board);
    addChild(_gameLayer);

    auto control = LocalFightControlLayer::create();
    addChild(control);

    /* response for reset */
    auto reset_cb = [this](EventCustom* ev) {
        removeAllChildren();
        auto scene = LocalFightScene::create();
        Director::getInstance()->replaceScene(scene);
    };
    /* response for save */
    auto save_cb = [this](EventCustom* ev) {
        UserData::SaveElement se;
        //se.type = TYPE_FIGHT;
        se.roleWhite = 0;
        se.roleBlack = 0;
        se.level = 0;
        se.white = "white";
        se.black = "black";
        se.content = _board->getFenWithMove();
        UserData::getInstance()->insertSaveElement(se);
    };

    auto over_cb = [this, origin, vsize](EventCustom *ev) {
        std::string event = (const char *)ev->getUserData();
        Sprite *sprite;

        if (event.find("DRAW:") != std::string::npos) {
            int count = UserData::getInstance()->getIntegerForKey(
                "LocalFightScene:DRAW", 0);
            UserData::getInstance()->setIntegerForKey(
                "LocalFightScene:DRAW", count+1);
            sprite = Sprite::create("board/watch_win_draw_tag.png");
        } else if (event.find("WIN:WHITE") != std::string::npos) {
            int count = UserData::getInstance()->getIntegerForKey(
                "LocalFightScene:WIN:WHITE", 0);
            UserData::getInstance()->setIntegerForKey(
                "LocalFightScene:WIN:WHITE", count+1);
            sprite = Sprite::create("board/watch_win_red_tag.png");
        } else {
            int count = UserData::getInstance()->getIntegerForKey(
                "LocalFightScene:WIN:BLACK", 0);
            UserData::getInstance()->setIntegerForKey(
                "LocalFightScene:WIN:BLACK", count+1);
            sprite = Sprite::create("board/watch_win_black_tag.png");
        }
        addChild(sprite);
        sprite->setPosition(origin.x + vsize.width/2, origin.y + vsize.height/2);
    };

    setOnEnterCallback([this, reset_cb, save_cb, over_cb]() {
            getEventDispatcher()->addCustomEventListener(EVENT_RESET, reset_cb);
            getEventDispatcher()->addCustomEventListener(EVENT_SAVE, save_cb);
            getEventDispatcher()->addCustomEventListener(EVENT_GAMEOVER, over_cb);
        });

    setOnExitCallback([this]() {
            getEventDispatcher()->removeCustomEventListeners(EVENT_RESET);
            getEventDispatcher()->removeCustomEventListeners(EVENT_SAVE);
            getEventDispatcher()->removeCustomEventListeners(EVENT_GAMEOVER);
        });

    return true;
}
