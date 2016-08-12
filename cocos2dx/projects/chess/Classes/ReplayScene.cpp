#include "ReplayScene.h"
#include "UserData.h"
#include "GameLayer.h"
#include "FightScene.h"

bool ReplayScene::init()
{
	if (!Scene::init())
		return false;

	auto vsize = Director::getInstance()->getVisibleSize();

	_tab = TabControl::create();
	//_tab->setContentSize(Size(vsize.width, vsize.height-200-200));
	_tab->setContentSize(vsize*2/3);
	_tab->setHeaderHeight(50.f);
	_tab->setHeaderWidth(200.f);
	_tab->ignoreHeadersTextureSize(false);
	_tab->setHeaderSelectedZoom(.1f);
	_tab->setHeaderDockPlace(TabControl::Dock::TOP);

	auto header1 = TabHeader::create();
	auto header2 = TabHeader::create();
	header1->loadTextureBackGround("ReplayScene/mysave_title.png");
	header2->loadTextureBackGround("ReplayScene/zuijin_title.png");

	auto container1 = VBox::create();
	auto container2 = VBox::create();

	_tab->insertTab(0, header1, container1);
	_tab->insertTab(1, header2, container2);

	_tab->setSelectTab(0);

	_tab->setTabChangedEventListener(
			[](int index, TabControl::EventType evtType)
	{
		log("tab %d selected", index);
	});

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
	_default_model->setSwallowTouches(false);

	_listview = ListView::create();
	_listview->setDirection(ScrollView::Direction::VERTICAL);
	_listview->setBounceEnabled(true);
	_listview->setScrollBarEnabled(false);
	_listview->setItemModel(_default_model);
	_listview->setGravity(ListView::Gravity::CENTER_HORIZONTAL);
	_listview->setItemsMargin(30);
	//XXX
	_listview->setSwallowTouches(false);
	//XXX
	_listview->setContentSize(container1->getContentSize());

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
			std::vector<UserData::SaveElement> saveElements;
			UserData::getInstance()->querySaveTbl(saveElements);

			auto e = saveElements[index];

			FightScene::Role roleWhite = (FightScene::Role)e.roleWhite;
			FightScene::Role roleBlack = (FightScene::Role)e.roleBlack;
			auto scene = FightScene::create(roleWhite, roleBlack, e.level, e.content);
			Director::getInstance()->replaceScene(scene);
		}
	default:
		break;
	}
	});

	setOnEnterCallback([&](){
		_listview->removeAllItems();

		std::vector<UserData::SaveElement> saveElements;
		UserData::getInstance()->querySaveTbl(saveElements);

		for(size_t i = 0; i < saveElements.size(); i++) {
			Widget* item = _default_model->clone();
			item->setTag(i);
			Button* btn = (Button*)item->getChildByName("Title Button");
			btn->setTitleFontSize(40);
			btn->setTitleText(saveElements[i].date);
			_listview->pushBackCustomItem(item);
		}
	});

	container1->addChild(_listview);

	auto vbox = VBox::create();
	vbox->setContentSize(vsize);
	vbox->setBackGroundImage("MainMenuScene/common_bg_new2.png");

	auto headview = Layout::create();
	headview->setContentSize(Size(vsize.width, 200));
	headview->setLayoutParameter(lp->clone());
	vbox->addChild(headview);

	_tab->setLayoutParameter(lp->clone());
	vbox->addChild(_tab);

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
