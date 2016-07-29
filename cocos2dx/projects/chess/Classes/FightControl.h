#ifndef __FIGHTCONTROL_H__
#define __FIGHTCONTROL_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "PopupBox.h"

USING_NS_CC;
using namespace cocos2d::ui;

class FightControl : public Layer
{
public:
	virtual bool init()
	{
		if (!Layer::init())
			return false;

		auto visibleSize = Director::getInstance()->getVisibleSize();
		Vec2 origin = Director::getInstance()->getVisibleOrigin();

		// +++++ back button
		auto backBtn = Button::create("FightSceneMenu/look_back.png");
		backBtn->setZoomScale(0.1);
		backBtn->addClickEventListener([](Ref *ref){
                Director::getInstance()->popScene();
            });
		backBtn->setPosition(Vec2(100, visibleSize.height-100));
		addChild(backBtn);
		// -----

		// +++++ menu
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

		_items.pushBack(MenuItemImage::create(
				"FightSceneMenu/game_button_save.png",
				"FightSceneMenu/game_button_saveP.png",
				"FightSceneMenu/game_button_saveDIS.png",
				[&](Ref *ref) {
					log("onSave");
					getEventDispatcher()->dispatchCustomEvent(EVENT_SAVE);

					auto text = Text::create("Save Success", "fonts/arial.ttf", 50);
					text->setTextColor(Color4B::BLACK);
                    auto box = PopupBox::create();
                    box->pushBackView(text);
                    addChild(box);

					auto action = Sequence::create(
                        DelayTime::create(3), RemoveSelf::create(), nullptr);
                    box->runAction(action->clone());
				}));
		_items.back()->setName("btn_save");
		_items.pushBack(MenuItemImage::create(
				"FightSceneMenu/game_button_lose_new.png",
				"FightSceneMenu/game_button_loseP_new.png",
				"FightSceneMenu/game_button_loseG_new.png",
				[&](Ref *ref) {
					log("onLose");
					getEventDispatcher()->dispatchCustomEvent(EVENT_RESIGN);
				}));
		_items.back()->setName("btn_lose");
		_items.pushBack(MenuItemImage::create(
				"FightSceneMenu/game_button_peace_new.png",
				"FightSceneMenu/game_button_peaceP_new.png",
				"FightSceneMenu/game_button_peaceG_new.png",
				[&](Ref *ref) {
					log("onPeace");
					getEventDispatcher()->dispatchCustomEvent(EVENT_TIP);
				}));
		_items.back()->setName("btn_peace");
		_items.pushBack(MenuItemImage::create(
				"FightSceneMenu/game_button_02.png",
				"FightSceneMenu/game_button_02P.png",
				"FightSceneMenu/game_button_02dis.png",
				[&](Ref *ref) {
					log("onReset");
					getEventDispatcher()->dispatchCustomEvent(EVENT_RESET);
				}));
		_items.back()->setName("btn_Reset");
		_items.pushBack(MenuItemImage::create(
				"FightSceneMenu/game_button_01.png",
				"FightSceneMenu/game_button_01P.png",
				"FightSceneMenu/game_button_01G.png",
				[&](Ref *ref) {
					log("onRegret");
					getEventDispatcher()->dispatchCustomEvent(EVENT_REGRET);
				}));
		_items.back()->setName("btn_regret");
		_items.pushBack(MenuItemImage::create(
				"FightSceneMenu/look_switch.png",
				"FightSceneMenu/look_switch_sel.png",
				"FightSceneMenu/look_switch_disable.png",
				[&](Ref *ref) {
					log("onChangeSide");
					getEventDispatcher()->dispatchCustomEvent(EVENT_SWITCH);
				}));
		_items.back()->setName("btn_switch");

		int n = _items.size();
		int i = 0;
		for (auto &item : _items) {
			auto bgPos = bg->getPosition();
			auto contentSize = bg->getContentSize();
			int offsetX = (contentSize.width/n) * (i + 0.5);
			item->setPositionX(bgPos.x - contentSize.width/2 + offsetX);
			item->setPositionY(bgPos.y);
			i++;
		}

		auto subMenu = Menu::createWithArray(_items);
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
		// -----

		auto over_callback = [this](EventCustom* ev){
			auto subMenu = dynamic_cast<Menu*>(_menuNode->getChildByName("subMenu"));
			auto btn_regret = dynamic_cast<MenuItemImage*>(subMenu->getChildByName("btn_regret"));
			auto btn_lose = dynamic_cast<MenuItemImage*>(subMenu->getChildByName("btn_lose"));
			auto btn_peace = dynamic_cast<MenuItemImage*>(subMenu->getChildByName("btn_peace"));
			btn_regret->setEnabled(false);
			btn_lose->setEnabled(false);
			btn_peace->setEnabled(false);

			auto vsize = Director::getInstance()->getVisibleSize();
			Vec2 origin = Director::getInstance()->getVisibleOrigin();

			std::string event = (const char *)ev->getUserData();

			if (event.find("WIN:WHITE") != std::string::npos) {
				auto s = Sprite::create("board/watch_win_red_tag.png");
				addChild(s);
				s->setPosition(origin.x + vsize.width/2,
						origin.y + vsize.height/2);
			} else if (event.find("WIN:BLACK") != std::string::npos) {
				auto s = Sprite::create("board/watch_win_black_tag.png");
				addChild(s);
				s->setPosition(origin.x + vsize.width/2,
						origin.y + vsize.height/2);
			} else if (event.find("DRAW:") != std::string::npos) {
				auto s = Sprite::create("board/watch_win_draw_tag.png");
				addChild(s);
				s->setPosition(origin.x + vsize.width/2,
						origin.y + vsize.height/2);
			}

			Device::vibrate(0.4);
		};

		setOnEnterCallback([this, listener, over_callback](){
			getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, this);
			getEventDispatcher()->addCustomEventListener(EVENT_GAMEOVER, over_callback);

			Device::setKeepScreenOn(true);
		});

		setOnExitCallback([this](){
			getEventDispatcher()->removeCustomEventListeners(EVENT_GAMEOVER);
			getEventDispatcher()->removeEventListenersForTarget(this);

			Device::setKeepScreenOn(false);
		});

		return true;
	}

	CREATE_FUNC(FightControl);

    void setEnabled(bool enable)
    {
        //auto subMenu = dynamic_cast<Menu*>(_menuNode->getChildByName("subMenu"));
        //subMenu->setEnabled(enable);
		for (auto &item : _items) {
			item->setEnabled(enable);
		}
    }

private:
	Node *_menuNode;
	Vector <MenuItem*> _items;
};

#endif
