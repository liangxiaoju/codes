#include "ChallengeScene.h"
#include "FightScene.h"
#include "GameLayer.h"
#include "BGLayer.h"
#include "AIPlayer.h"
#include "ControlLayer.h"
#include "HeaderSprite.h"
#include "Sound.h"
#include "UserData.h"
#include "Localization.h"
#include "ChallengeMap.h"

bool ChallengeScene::init(EndGameData::EndGameItem item)
{
	if (!Scene::init())
		return false;

	_item = item;
	std::string fen = item.data.fen;

	auto vsize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	auto bgLayer = BGLayer::create();
	addChild(bgLayer);

	auto board = Board::createWithFen(fen);
	auto playerWhite = UIPlayer::create();
	playerWhite->setBoard(board);
	auto playerBlack = AIPlayer::create();

	auto lheader = HeaderSprite::createWithType(HeaderSprite::Type::LEFT);
	auto rheader = HeaderSprite::createWithType(HeaderSprite::Type::RIGHT);
	auto lsize = lheader->getContentSize();
	auto rsize = rheader->getContentSize();
	lheader->setPosition(origin.x+vsize.width/2, origin.y+lsize.height/2+20);
	rheader->setPosition(origin.x+vsize.width/2, origin.y+vsize.height-rsize.height/2-20);
	rheader->setNameLine(item.data.subtitle);

	auto gameLayer = GameLayer::create(playerWhite, playerBlack, board);
	gameLayer->addChild(lheader);
	gameLayer->addChild(rheader);
	addChild(gameLayer);

	/* active/deactive header */
	auto white_start_cb = [this, lheader, rheader](EventCustom* ev){
		lheader->setActive(true);
		rheader->setActive(false);
	};
	auto black_start_cb = [this, lheader, rheader](EventCustom* ev){
		rheader->setActive(true);
		lheader->setActive(false);
	};

	auto control = ChallengeControlLayer::create();
	addChild(control);
	//control->setEndGameItem(item);

	/* response for reset */
	auto reset_cb = [this](EventCustom* ev) {
		removeAllChildren();
		auto scene = ChallengeScene::create(_item);
		Director::getInstance()->replaceScene(scene);
	};
	/* response for next */
	auto next_cb = [this](EventCustom* ev) {
		removeAllChildren();

		std::vector<EndGameData::EndGameItem> endGameItems;
		EndGameData::getInstance()->queryEndGameItem(_item.tid, endGameItems);
		if (_item.sort == (endGameItems.size()-1)) {
			Director::getInstance()->popScene();
		} else {
			_item = endGameItems[_item.sort+1];
			auto scene = ChallengeScene::create(_item);
			Director::getInstance()->replaceScene(scene);
		}
	};
    auto over_cb = [this, origin, vsize](EventCustom *ev) {
        std::string event = (const char *)ev->getUserData();
        Sprite *sprite;

        auto white_win_callback = [&](){
            std::vector<EndGameData::EndGameClass> endGameClass;
            EndGameData::getInstance()->queryEndGameClass(endGameClass);
            for (auto &cls : endGameClass) {
                if (cls.tid == _item.tid) {
                    if (cls.progress == _item.sort) {
                        cls.progress = _item.sort+1;
                        EndGameData::getInstance()->updateEndGameClass(cls);
                    }
                    break;
                }
            }
            auto next = Button::create("ChallengeScene/button_next.png");
            next->setTitleText(TR("Next"));
            next->setTitleFontSize(35);
            next->setZoomScale(0.1);
            next->addClickEventListener([&](Ref *ref){
                    getEventDispatcher()->dispatchCustomEvent(EVENT_NEXT);
                });
            auto vsize = Director::getInstance()->getVisibleSize();
            next->setPosition(Vec2(vsize.width/2, vsize.height/3));
            addChild(next);
        };

        if (event.find("DRAW:") != std::string::npos) {
            Sound::getInstance()->playEffect("draw");
            sprite = Sprite::create("ChallengeScene/text_hq.png");
        } else if (event.find("WIN:WHITE") != std::string::npos) {
            Sound::getInstance()->playEffect("win");
            UserData::getInstance()->setIntegerForKey("ChallengeScene:LEVEL", _item.id);
            sprite = Sprite::create("ChallengeScene/text_yq.png");
            white_win_callback();
        } else {
            Sound::getInstance()->playEffect("lose");
            sprite = Sprite::create("ChallengeScene/text_sq.png");
        }
        addChild(sprite);
        sprite->setPosition(origin.x + vsize.width/2, origin.y + vsize.height/2);
    };

	setOnEnterCallback([this, reset_cb, next_cb, white_start_cb, black_start_cb, over_cb](){
		getEventDispatcher()->addCustomEventListener(EVENT_RESET, reset_cb);
		getEventDispatcher()->addCustomEventListener(EVENT_NEXT, next_cb);
		getEventDispatcher()->addCustomEventListener(EVENT_WHITE_START, white_start_cb);
		getEventDispatcher()->addCustomEventListener(EVENT_BLACK_START, black_start_cb);
        getEventDispatcher()->addCustomEventListener(EVENT_GAMEOVER, over_cb);
	});

	setOnExitCallback([this](){
		getEventDispatcher()->removeCustomEventListeners(EVENT_RESET);
		getEventDispatcher()->removeCustomEventListeners(EVENT_NEXT);
		getEventDispatcher()->removeCustomEventListeners(EVENT_WHITE_START);
		getEventDispatcher()->removeCustomEventListeners(EVENT_BLACK_START);
		getEventDispatcher()->removeCustomEventListeners(EVENT_GAMEOVER);
	});

	return true;
}

bool ChallengeMapScene::init()
{
    if (!Scene::init())
        return false;

    auto vsize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    std::vector<EndGameData::EndGameClass> classes;
    EndGameData::getInstance()->queryEndGameClass(classes);

    int tid = classes[0].tid;
    int total = classes[0].subCount;
    int progress = classes[0].progress;

    auto map = ChallengeMap::create(total);
    map->addClickEventListener([tid](int index) {
            std::vector<EndGameData::EndGameItem> items;
            EndGameData::getInstance()->queryEndGameItem(tid, items);
            auto scene = ChallengeScene::create(items[index]);
            Director::getInstance()->pushScene(scene);
            });
    addChild(map);
    map->setProgress(progress);
    map->jumpToProgress(progress);

	auto back = Button::create("DifficultyScene/look_back.png");
	back->addClickEventListener([](Ref *ref){
			Director::getInstance()->popScene();
			});
	back->setZoomScale(0.1);
	back->setPosition(Vec2(100, vsize.height-100));
	addChild(back);

    setOnEnterCallback([this, map]() {
            std::vector<EndGameData::EndGameClass> classes;
            EndGameData::getInstance()->queryEndGameClass(classes);
            int progress = classes[0].progress;
            if (progress != map->getProgress()) {
                map->setProgress(progress);
                map->jumpToProgress(progress);
            }
            });

    return true;
}

