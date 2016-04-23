#include "ReplayScene.h"
#include "UserData.h"
#include "GameLayer.h"
#include "FightScene.h"

bool ReplayScene::init()
{
	if (!Scene::init())
		return false;

	auto vsize = Director::getInstance()->getVisibleSize();

	auto tab = TabControl::create();
	tab->setContentSize(vsize);
	tab->setHeaderHeight(50.f);
	tab->setHeaderWidth(200.f);
	tab->setHeaderSelectedZoom(.1f);
	tab->setHeaderDockPlace(TabControl::Dock::TOP);

	auto header1 = TabHeader::create();
	header1->loadTextureBackGround("ReplayScene/mysave_title.png");
	auto header2 = TabHeader::create();
	header2->loadTextureBackGround("ReplayScene/zuijin_title.png");

	auto container1 = VBox::create();
	container1->setOpacity(255);
	container1->setBackGroundColorType(Layout::BackGroundColorType::SOLID);
	container1->setBackGroundColor(Color3B::GRAY);
	container1->setBackGroundColorOpacity(255);
	auto container2 = VBox::create();
	container2->setBackGroundColorType(Layout::BackGroundColorType::SOLID);
	container2->setOpacity(255);
	container2->setBackGroundColor(Color3B::BLUE);
	container2->setBackGroundColorOpacity(255);

	tab->insertTab(0, header1, container1);
	tab->insertTab(1, header2, container2);

	tab->setSelectTab(0);
	addChild(tab);
	tab->setPosition(vsize/2);

	tab->setTabChangedEventListener(
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

	_listview = ListView::create();
	_listview->setDirection(ScrollView::Direction::VERTICAL);
	_listview->setBounceEnabled(true);
	_listview->setScrollBarEnabled(false);
	_listview->setItemModel(_default_model);
	_listview->setGravity(ListView::Gravity::CENTER_HORIZONTAL);
	_listview->setItemsMargin(30);

	container1->addChild(_listview);

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
		}
	default:
		break;
	}
	});

	setOnEnterCallback([&](){
		_listview->removeAllItems();

		std::vector<UserData::SaveElement> saveElements;
		UserData::getInstance()->querySaveTbl(saveElements);

		for(int i = 0; i < saveElements.size(); i++) {
			Widget* item = _default_model->clone();
			item->setTag(i);
			Button* btn = (Button*)item->getChildByName("Title Button");
			btn->setTitleFontSize(40);
			btn->setTitleText(saveElements[i].date);
			_listview->pushBackCustomItem(item);
		}
	});

	return true;
}
