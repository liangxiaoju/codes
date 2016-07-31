#include "ChallengeScene.h"
#include "FightScene.h"
#include "GameLayer.h"
#include "BGLayer.h"
#include "AIPlayer.h"
#include "ChallengeControl.h"
#include "HeaderSprite.h"
#include "Sound.h"

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

	auto control = ChallengeControl::create();
	addChild(control);
	control->setEndGameItem(item);

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
    auto over_cb = [this](EventCustom *ev) {
        std::string event = (const char *)ev->getUserData();
        if (event.find("DRAW:") != std::string::npos) {
            Sound::getInstance()->playEffect("draw");
        } else if (event.find("WIN:WHITE") != std::string::npos) {
            Sound::getInstance()->playEffect("win");
        } else {
            Sound::getInstance()->playEffect("lose");
        }
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

bool ChallengeMenuL2::init(EndGameData::EndGameClass cls)
{
	if (!Scene::init())
		return false;

	_endGameClass = cls;

	auto vsize = Director::getInstance()->getVisibleSize();

	LinearLayoutParameter* lp = LinearLayoutParameter::create();
	lp->setGravity(LinearLayoutParameter::LinearGravity::CENTER_HORIZONTAL);

	// _listview model
	Button *default_button = Button::create("ChallengeScene/buttonPost.png");
	default_button->setName("Title Button");
	default_button->setZoomScale(0.1);
	default_button->setLayoutParameter(lp->clone());
	_default_model= RelativeBox::create();
	_default_model->setTouchEnabled(true);
	_default_model->setContentSize(default_button->getContentSize());
	_default_model->addChild(default_button);

	_listview = ListView::create();
	_listview->setDirection(ScrollView::Direction::VERTICAL);
	_listview->setBounceEnabled(true);
	_listview->setScrollBarEnabled(false);
	_listview->setItemModel(_default_model);
	_listview->setGravity(ListView::Gravity::CENTER_HORIZONTAL);
	_listview->setItemsMargin(30);

	_listview->addEventListener([&](Ref *pSender, ListView::EventType type){
	switch (type)
	{
	case ListView::EventType::ON_SELECTED_ITEM_START:
		{
			ListView* listView = static_cast<ListView*>(pSender);
			auto item = listView->getItem(listView->getCurSelectedIndex());
			CCLOG("select child start index = %d", item->getTag());
			break;
		}
	case ListView::EventType::ON_SELECTED_ITEM_END:
		{
			ListView* listView = static_cast<ListView*>(pSender);
			auto item = listView->getItem(listView->getCurSelectedIndex());
			CCLOG("select child end index = %d", item->getTag());

			int index = item->getTag();
			Button* btn = (Button*)item->getChildByName("Title Button");

			if (btn->isEnabled()) {
				log("create FightScene with feh: %s", _endGameItems[index].data.fen.c_str());
				auto scene = ChallengeScene::create(_endGameItems[index]);
				Director::getInstance()->pushScene(scene);
			}
		}
	default:
		break;
	}
	});

	setOnEnterCallback([&](){
		_endGameItems.clear();
		_listview->removeAllItems();
		std::vector<EndGameData::EndGameClass> endGameClass;
		EndGameData::getInstance()->queryEndGameClass(endGameClass);
		for (auto &cls : endGameClass) {
			if (cls.tid == _endGameClass.tid) {
				_endGameClass = cls;
			}
		}
		int tid = _endGameClass.tid;
		int progress = _endGameClass.progress;
		EndGameData::getInstance()->queryEndGameItem(tid, _endGameItems);
		log("endGameItems size: %d", _endGameItems.size());

		//initial the data
		for (int i = 0; i < _endGameItems.size(); ++i) {
			Widget* item = _default_model->clone();
			item->setTag(i);
			Button* btn = (Button*)item->getChildByName("Title Button");
			btn->setTitleFontSize(40);
			btn->setTitleFontName("");
			btn->setTitleText(_endGameItems[i].data.subtitle);

			if (i < (progress+1)) {
				btn->setEnabled(true);
			} else {
				btn->setEnabled(false);
			}

			if (i < progress) {
				ImageView *image = ImageView::create(
						"ChallengeScene/challenge_success.png");
				item->addChild(image);
			}
			_listview->pushBackCustomItem(item);
		}
	});

	auto vbox = VBox::create();
	_headview = Layout::create();

	vbox->setContentSize(vsize);
	_headview->setContentSize(Size(vsize.width, 200));
	_listview->setContentSize(Size(vsize.width, vsize.height-200-100));

	_headview->setLayoutParameter(lp->clone());
	_listview->setLayoutParameter(lp->clone());

	vbox->addChild(_headview);
	vbox->addChild(_listview);

	vbox->setBackGroundImage("MainMenuScene/common_bg_new2.png");
	addChild(vbox);

	auto back = Button::create("DifficultyScene/look_back.png");
	back->addClickEventListener([](Ref *ref){
			Director::getInstance()->popScene();
			});
	back->setZoomScale(0.1);
	back->setPosition(Vec2(100, vsize.height-100));
	addChild(back);

	return true;
}

bool ChallengeMenuL1::init()
{
	if (!Scene::init())
		return false;

	auto vsize = Director::getInstance()->getVisibleSize();

	LinearLayoutParameter* lp = LinearLayoutParameter::create();
	lp->setGravity(LinearLayoutParameter::LinearGravity::CENTER_HORIZONTAL);

	// _listview model
	Button *default_button = Button::create("ChallengeScene/tga_title_fat.png");
	default_button->setName("Title Button");
	default_button->setZoomScale(0.1);
	default_button->setLayoutParameter(lp->clone());
	_default_model= HBox::create();
	_default_model->setTouchEnabled(true);
	_default_model->setContentSize(default_button->getContentSize());
	_default_model->addChild(default_button);

	_listview = ListView::create();
	_listview->setDirection(ScrollView::Direction::VERTICAL);
	_listview->setBounceEnabled(true);
	_listview->setScrollBarEnabled(false);
	_listview->setItemModel(_default_model);
	_listview->setGravity(ListView::Gravity::CENTER_HORIZONTAL);
	_listview->setItemsMargin(50);

	_listview->addEventListener([&](Ref *pSender, ListView::EventType type){
	switch (type)
	{
		case ListView::EventType::ON_SELECTED_ITEM_START:
			{
				ListView* listView = static_cast<ListView*>(pSender);
				auto item = listView->getItem(listView->getCurSelectedIndex());
				CCLOG("select child start index = %d", item->getTag());
				break;
			}
		case ListView::EventType::ON_SELECTED_ITEM_END:
			{
				ListView* listView = static_cast<ListView*>(pSender);
				auto item = listView->getItem(listView->getCurSelectedIndex());
				CCLOG("select child end index = %d", item->getTag());

				int index = item->getTag();
				int tid = _endGameClass[index].tid;
				Button* btn = (Button*)item->getChildByName("Title Button");

				if (btn->isEnabled()) {
					auto scene = ChallengeMenuL2::create(_endGameClass[index]);
					Director::getInstance()->pushScene(scene);
				}

				break;
			}
		default:
			break;
	}
	});

	setOnEnterCallback([&](){
		_endGameClass.clear();
		_listview->removeAllItems();
		EndGameData::getInstance()->queryEndGameClass(_endGameClass);
		//initial the data
		for (int i = 0; i < _endGameClass.size(); ++i) {
			Widget* item = _default_model->clone();
			item->setTag(i);
			Button* btn = (Button*)item->getChildByName("Title Button");
			btn->setTitleFontSize(40);
			btn->setTitleFontName("");
			btn->setTitleText(_endGameClass[i].data.title);

			if ((i > 0) && (_endGameClass[i-1].progress < _endGameClass[i-1].subCount)) {
					btn->setEnabled(false);
			}

			_listview->pushBackCustomItem(item);
		}
	});

	auto vbox = VBox::create();
	_headview = Layout::create();

	vbox->setContentSize(vsize);
	_headview->setContentSize(Size(vsize.width, 200));
	_listview->setContentSize(Size(vsize.width, vsize.height-200-100));

	_headview->setLayoutParameter(lp->clone());
	_listview->setLayoutParameter(lp->clone());

	vbox->addChild(_headview);
	vbox->addChild(_listview);

	vbox->setBackGroundImage("MainMenuScene/common_bg_new2.png");
	addChild(vbox);

	auto back = Button::create("DifficultyScene/look_back.png");
	back->addClickEventListener([](Ref *ref){
			Director::getInstance()->popScene();
			});
	back->setZoomScale(0.1);
	back->setPosition(Vec2(100, vsize.height-100));
	addChild(back);

	return true;
}

