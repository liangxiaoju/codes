#include "MainMenuScene.h"
#include "FightScene.h"
#include "ReplayScene.h"
#include "ChallengeScene.h"
#include "LANFightScene.h"
#include "TutorialScene.h"

class DifficultyScene : public Scene
{
public:
	virtual bool init()
	{
		if (!Scene::init())
			return false;

		auto visibleSize = Director::getInstance()->getVisibleSize();

		auto b1 = Button::create("DifficultyScene/NG_button_primary_new.png");
		auto b2 = Button::create("DifficultyScene/NG_button_mid_new.png");
		auto b3 = Button::create("DifficultyScene/NG_button_Senior_new.png");
		auto b4 = Button::create("DifficultyScene/NG_button_superfine_new.png");

		b1->setZoomScale(0.1);
		b2->setZoomScale(0.1);
		b3->setZoomScale(0.1);
		b4->setZoomScale(0.1);

		LinearLayoutParameter* lp1 = LinearLayoutParameter::create();
		lp1->setGravity(LinearLayoutParameter::LinearGravity::CENTER_HORIZONTAL);

		b4->setLayoutParameter(lp1->clone());
		b3->setLayoutParameter(lp1->clone());
		b2->setLayoutParameter(lp1->clone());
		lp1->setMargin(Margin(0.0f, 150.0f, 0.0f, 0.0f));
		b1->setLayoutParameter(lp1);

		b1->addClickEventListener([](Ref *ref){
			auto scene = FightScene::create(FightScene::UI, FightScene::AI, 0);
			Director::getInstance()->pushScene(scene);
		});
		b2->addClickEventListener([](Ref *ref){
			auto scene = FightScene::create(FightScene::UI, FightScene::AI, 2);
			Director::getInstance()->pushScene(scene);
		});
		b3->addClickEventListener([](Ref *ref){
			auto scene = FightScene::create(FightScene::UI, FightScene::AI, 5);
			Director::getInstance()->pushScene(scene);
		});
		b4->addClickEventListener([](Ref *ref){
			auto scene = FightScene::create(FightScene::UI, FightScene::UI, 8);
			Director::getInstance()->pushScene(scene);
		});

		auto vbox = VBox::create();
		vbox->setBackGroundImage("MainMenuScene/common_bg_new2.png");

		vbox->addChild(b1);
		vbox->addChild(b2);
		vbox->addChild(b3);
		vbox->addChild(b4);

		vbox->setContentSize(visibleSize);
		addChild(vbox);

		auto back = Button::create("DifficultyScene/look_back.png");
		back->addClickEventListener([](Ref *ref){
			Director::getInstance()->popScene();
		});
		back->setZoomScale(0.1);
		back->setPosition(Vec2(100, visibleSize.height-100));
		addChild(back);

		return true;
	}

	CREATE_FUNC(DifficultyScene);
};

bool MainMenuLayer::init()
{
	if (!Layer::init())
		return false;

	auto visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	auto b1 = Button::create("MainMenuScene/main_button1.png");
	auto b2 = Button::create("MainMenuScene/main_button2.png");
	auto b3 = Button::create("MainMenuScene/main_button3.png");
	auto b4 = Button::create("MainMenuScene/main_newbutton0.png");

	b1->setScale(1.1);
	b2->setScale(1.1);
	b3->setScale(1.1);
	b4->setScale(1.1);

	b1->addClickEventListener([](Ref *ref){
		Director::getInstance()->pushScene(DifficultyScene::create());
	});
	b2->addClickEventListener([](Ref *ref){
		Director::getInstance()->pushScene(ChallengeMenuL1::create());
	});
	b3->addClickEventListener([](Ref *ref){
            //Director::getInstance()->pushScene(ReplayScene::create());
        Director::getInstance()->pushScene(TutorialMenuScene::create());
	});
	b4->addClickEventListener([](Ref *ref){
		Director::getInstance()->pushScene(LANFightScene::create());
	});

	b1->setZoomScale(0.1);
	b2->setZoomScale(0.1);
	b3->setZoomScale(0.1);
	b4->setZoomScale(0.1);

	RelativeLayoutParameter* rp_tl = RelativeLayoutParameter::create();
	rp_tl->setAlign(RelativeLayoutParameter::RelativeAlign::PARENT_TOP_LEFT);
	b1->setLayoutParameter(rp_tl);

	RelativeLayoutParameter* rp_tr = RelativeLayoutParameter::create();
	rp_tr->setAlign(RelativeLayoutParameter::RelativeAlign::PARENT_TOP_RIGHT);
	b2->setLayoutParameter(rp_tr);

	RelativeLayoutParameter* rp_lb = RelativeLayoutParameter::create();
	rp_lb->setAlign(RelativeLayoutParameter::RelativeAlign::PARENT_LEFT_BOTTOM);
	b3->setLayoutParameter(rp_lb);

	RelativeLayoutParameter* rp_rb = RelativeLayoutParameter::create();
	rp_rb->setAlign(RelativeLayoutParameter::RelativeAlign::PARENT_RIGHT_BOTTOM);
	b4->setLayoutParameter(rp_rb);

	auto layout1 = Layout::create();
	layout1->setLayoutType(Layout::Type::HORIZONTAL);
	layout1->setBackGroundImage("MainMenuScene/hall_loading_loading_logo.png");
	layout1->setContentSize(Size(720, 450));

	LinearLayoutParameter* lp1 = LinearLayoutParameter::create();
	layout1->setLayoutParameter(lp1);
	lp1->setGravity(LinearLayoutParameter::LinearGravity::CENTER_HORIZONTAL);
	lp1->setMargin(Margin(0.0f, 100.0f, 0.0f, 10.0f));

	auto layout2 = Layout::create();
	layout2->setLayoutType(Layout::Type::RELATIVE);
	layout2->setContentSize(Size(450, 450));

	LinearLayoutParameter* lp2 = LinearLayoutParameter::create();
	layout2->setLayoutParameter(lp2);
	lp2->setGravity(LinearLayoutParameter::LinearGravity::CENTER_HORIZONTAL);
	lp2->setMargin(Margin(0.0f, 10.0f, 0.0f, 10.0f));

	auto layout3 = Layout::create();
	layout3->setLayoutType(Layout::Type::RELATIVE);
	layout3->setContentSize(Size(visibleSize.width, 80));

	LinearLayoutParameter* lp3 = LinearLayoutParameter::create();
	lp3->setGravity(LinearLayoutParameter::LinearGravity::CENTER_HORIZONTAL);
	//lp3->setMargin(Margin(30.0f, 20.0f, 30.0f, 20.0f));
	layout3->setLayoutParameter(lp3);

	//auto setting = Button::create("MainMenuScene/main_button_setting.png", "MainMenuScene/main_button_settingP.png");
	auto setting = Button::create("MainMenuScene/main_button_setting.png");
	//setting->setScale(1.2);
	setting->addClickEventListener([](Ref *ref){
            log("setting click");
            auto box = PopupBox::create();
        });
	setting->setZoomScale(0.1);

	RelativeLayoutParameter* rp_r = RelativeLayoutParameter::create();
	rp_r->setAlign(RelativeLayoutParameter::RelativeAlign::PARENT_RIGHT_CENTER_VERTICAL);
	rp_r->setMargin(Margin(0.0f, 0.0f, 30.0f, 0.0f));
	setting->setLayoutParameter(rp_r);

	auto quit = Button::create("MainMenuScene/common_back.png");
	//quit->setScale(1.2);
	quit->addClickEventListener([this](Ref *ref){
		log("quit click");
        auto box = DialogBox::create(
            "Are you sure to exit ?", "Yes", "No", [this](bool yes){
                removeChildByName("box");
                if (yes) {
                    Director::getInstance()->end();
                }
            });
        addChild(box, 1, "box");
	});
	quit->setZoomScale(0.1);

	RelativeLayoutParameter* rp_l = RelativeLayoutParameter::create();
	rp_l->setMargin(Margin(30.0f, 0.0f, 0.0f, 0.0f));
	rp_l->setAlign(RelativeLayoutParameter::RelativeAlign::PARENT_LEFT_CENTER_VERTICAL);
	quit->setLayoutParameter(rp_l);

	layout2->addChild(b1);
	layout2->addChild(b2);
	layout2->addChild(b3);
	layout2->addChild(b4);

	//layout3->addChild(setting);
	//layout3->addChild(quit);

	auto vbox = VBox::create();
	vbox->setBackGroundImage("MainMenuScene/common_bg_new2.png");
	vbox->addChild(layout1);
	vbox->addChild(layout2);
	//vbox->addChild(layout3);

	vbox->setContentSize(visibleSize);
	addChild(vbox);

	setting->setPosition(Vec2(visibleSize.width-100, visibleSize.height-100));
	addChild(setting);
	quit->setPosition(Vec2(100, visibleSize.height-100));
	addChild(quit);

	return true;
}

bool MainMenuScene::init()
{
	if (!Scene::init())
		return false;

	auto layer = MainMenuLayer::create();
	addChild(layer);

	return true;
}
