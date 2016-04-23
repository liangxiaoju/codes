#ifndef __CHALLENGECONTROL_H__
#define __CHALLENGECONTROL_H__

#include "cocos2d.h"
#include "GameLayer.h"

USING_NS_CC;

class ChallengeControl : public Layer
{
public:
	virtual bool init()
	{
		if (!Layer::init())
			return false;

		auto visibleSize = Director::getInstance()->getVisibleSize();
		Vec2 origin = Director::getInstance()->getVisibleOrigin();

		auto backBtn = Button::create("FightSceneMenu/look_back.png");
		backBtn->setZoomScale(0.1);
		backBtn->addClickEventListener([](Ref *ref){
			Director::getInstance()->popScene();
		});
		backBtn->setPosition(Vec2(origin.x+50, origin.y+visibleSize.height-120));
		addChild(backBtn);

		_menuNode = Node::create();

		auto menuItemBtn = MenuItemImage::create(
				"FightSceneMenu/common_push.png",
				"FightSceneMenu/common_pushP.png",
				"FightSceneMenu/common_push_dis.png",
				[&](Ref *ref) {
					_menuNode->setVisible(!_menuNode->isVisible());
				});
		auto btnSize = menuItemBtn->getContentSize();
		menuItemBtn->setPositionX(origin.x + visibleSize.width - btnSize.width/2 - 40);
		menuItemBtn->setPositionY(origin.y + btnSize.height/2+60);

		auto menuBtn = Menu::create(menuItemBtn, NULL);
		menuBtn->setPosition(Vec2::ZERO);
		addChild(menuBtn);

		auto bg = Sprite::create("FightSceneMenu/game_button_bg_new.png");
		auto bgSize = bg->getContentSize();
		bg->setPositionX(visibleSize.width-bgSize.width/2-20);
		bg->setPositionY(menuItemBtn->getPositionY()+btnSize.height/2+bgSize.height/2+20);
		_menuNode->addChild(bg, 0, "menuBG");

		Vector <MenuItem*> items;
		items.pushBack(MenuItemImage::create(
				"FightSceneMenu/game_button_lose_new.png",
				"FightSceneMenu/game_button_loseP_new.png",
				"FightSceneMenu/game_button_loseG_new.png",
				[&](Ref *ref) {
					log("onLose");
					_gameLayer->requestLose();
				}));
		items.back()->setName("btn_lose");
		items.pushBack(MenuItemImage::create(
				"FightSceneMenu/game_button_02.png",
				"FightSceneMenu/game_button_02P.png",
				"FightSceneMenu/game_button_02dis.png",
				[&](Ref *ref) {
					log("onReset");
					_gameLayer->removeFromParent();
					auto scene = ChallengeScene::create(_endGameItem);
					Director::getInstance()->replaceScene(scene);
				}));
		items.back()->setName("btn_Reset");
		items.pushBack(MenuItemImage::create(
				"FightSceneMenu/game_button_01.png",
				"FightSceneMenu/game_button_01P.png",
				"FightSceneMenu/game_button_01G.png",
				[&](Ref *ref) {
					log("onRegret");
					_gameLayer->requestRegret();
				}));
		items.back()->setName("btn_regret");

		int n = items.size();
		int i = 0;
		for (auto &item : items) {
			auto bgPos = bg->getPosition();
			auto contentSize = bg->getContentSize();
			int offsetX = (contentSize.width/n) * (i + 0.5);
			item->setPositionX(bgPos.x - contentSize.width/2 + offsetX);
			item->setPositionY(bgPos.y);
			i++;
		}

		auto subMenu = Menu::createWithArray(items);
		subMenu->setPosition(Vec2::ZERO);
		_menuNode->addChild(subMenu, 0, "subMenu");

		addChild(_menuNode);
		_menuNode->setVisible(false);

		auto listener = EventListenerTouchOneByOne::create();
		listener->onTouchBegan = [&](Touch *t, Event *e){
			if (!_menuNode->isVisible())
				return false;

			Vec2 touchPos = t->getLocation();
			Rect menuRect = _menuNode->getChildByName("menuBG")
				->getBoundingBox();

			if (!menuRect.containsPoint(touchPos)) {
				_menuNode->setVisible(false);
				return true;
			} else {
				return false;
			}
		};
		/* if onTouchBegan==true,
		 * then do not pass the event into low layers */
		listener->setSwallowTouches(true);
		getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener,this);

		auto over_callback = [this](EventCustom* ev){
			auto subMenu = dynamic_cast<Menu*>(_menuNode->getChildByName("subMenu"));
			auto btn_regret = dynamic_cast<MenuItemImage*>(subMenu->getChildByName("btn_regret"));
			auto btn_lose = dynamic_cast<MenuItemImage*>(subMenu->getChildByName("btn_lose"));
			btn_regret->setEnabled(false);
			btn_lose->setEnabled(false);
		};
		/* have to remove previous listener */
		getEventDispatcher()->removeCustomEventListeners(EVENT_GAMEOVER);
		getEventDispatcher()->addCustomEventListener(EVENT_GAMEOVER, over_callback);

		auto ui_go_callback = [this](EventCustom* ev){
			auto subMenu = dynamic_cast<Menu*>(_menuNode->getChildByName("subMenu"));
			auto btn_regret = dynamic_cast<MenuItemImage*>(subMenu->getChildByName("btn_regret"));
			btn_regret->setEnabled(true);
		};
		getEventDispatcher()->removeCustomEventListeners(EVENT_UIPLAYER_GO);
		getEventDispatcher()->addCustomEventListener(EVENT_UIPLAYER_GO, ui_go_callback);

		auto ai_go_callback = [this](EventCustom* ev){
			auto subMenu = dynamic_cast<Menu*>(_menuNode->getChildByName("subMenu"));
			auto btn_regret = dynamic_cast<MenuItemImage*>(subMenu->getChildByName("btn_regret"));
			btn_regret->setEnabled(false);
		};
		getEventDispatcher()->removeCustomEventListeners(EVENT_AIPLAYER_GO);
		getEventDispatcher()->addCustomEventListener(EVENT_AIPLAYER_GO, ai_go_callback);

		auto white_win_callback = [&](EventCustom* ev){
			std::vector<EndGameData::EndGameClass> endGameClass;
			EndGameData::getInstance()->queryEndGameClass(endGameClass);
			for (auto &cls : endGameClass) {
				if (cls.tid == _endGameItem.tid) {
					if (cls.progress == _endGameItem.sort) {
						cls.progress = _endGameItem.sort+1;
						EndGameData::getInstance()->updateEndGameClass(cls);
					}
					break;
				}
			}
			auto close = Button::create("ChallengeScene/button_next.png");
			close->setTitleText("Next");
			close->setTitleFontSize(35);
			close->setZoomScale(0.1);
			close->addClickEventListener([&](Ref *ref){
				_gameLayer->removeFromParent();
				std::vector<EndGameData::EndGameItem> endGameItems;
				EndGameData::getInstance()->queryEndGameItem(_endGameItem.tid, endGameItems);
				if (_endGameItem.sort == (endGameItems.size()-1)) {
					Director::getInstance()->popScene();
				} else {
					_endGameItem = endGameItems[_endGameItem.sort+1];
					auto scene = ChallengeScene::create(_endGameItem);
					Director::getInstance()->replaceScene(scene);
				}
			});
			auto vsize = Director::getInstance()->getVisibleSize();
			close->setPosition(Vec2(vsize.width/2, vsize.height/3));
			addChild(close);
		};
		getEventDispatcher()->removeCustomEventListeners(EVENT_WHITE_WIN);
		getEventDispatcher()->addCustomEventListener(EVENT_WHITE_WIN, white_win_callback);

		return true;
	}

	CREATE_FUNC(ChallengeControl);

	void setGameLayer(GameLayer *layer) { _gameLayer = layer; }
	void setEndGameItem(EndGameData::EndGameItem item) { _endGameItem = item; }

private:
	GameLayer *_gameLayer;
	EndGameData::EndGameItem _endGameItem;
	Node *_menuNode;
};

#endif
