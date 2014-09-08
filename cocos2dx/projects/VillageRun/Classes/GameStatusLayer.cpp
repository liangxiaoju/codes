#include "GameStatusLayer.h"
#include "GamePauseLayer.h"

bool GameStatusLayer::init() {
	bool bRet = false;

	do {
		CC_BREAK_IF(!Layer::init());

		Size s = Director::getInstance()->getVisibleSize();

        Sprite *s1_1 = Sprite::createWithSpriteFrameName("buttonPause.png");
        Sprite *s1_2 = Sprite::createWithSpriteFrameName("buttonPause.png");
		s1_2->setOpacity(160);
        MenuItemSprite *pause = MenuItemSprite::create(
                s1_1, s1_2, CC_CALLBACK_1(GameStatusLayer::pauseCallback, this));

        auto menu = Menu::create(pause, nullptr);
        menu->alignItemsHorizontallyWithPadding(50);
		menu->setPosition(s1_1->getContentSize().width/2, s.height-s1_1->getContentSize().height/2);
        addChild(menu);

		mScoreLabel = Label::createWithBMFont("fonts/score.fnt", "0");
		//mScoreLabel = Label::createWithSystemFont("0", "arial", 50);
		mScoreLabel->setColor(Color3B::BLACK);
		mScoreLabel->setAnchorPoint(Vec2(1, 1));
		mScoreLabel->setPosition(s.width-10, s.height-10);
		addChild(mScoreLabel);

		mScore = 0;

		CallFunc *callback = CallFunc::create([=]() {
			addGameScore(1);
		});
		auto seq = Sequence::create(DelayTime::create(0.05f), callback, nullptr);
		runAction(RepeatForever::create(seq));

		bRet = true;
	} while(0);

	return bRet;
}

void GameStatusLayer::onEnter() {
	Layer::onEnter();

	mListener = EventListenerCustom::create("EVENT_GameOver", [=](EventCustom* event){
		stopAllActions();

		int highestScore = UserDefault::getInstance()->getIntegerForKey("highestScore", 0);
		highestScore = highestScore > mScore ? highestScore : mScore;

		UserDefault::getInstance()->setIntegerForKey("highestScore", highestScore);
		UserDefault::getInstance()->setIntegerForKey("currentScore", mScore);
		UserDefault::getInstance()->flush();
	});
	_eventDispatcher->addEventListenerWithFixedPriority(mListener, 1);
}

void GameStatusLayer::onExit() {
	Layer::onExit();
	_eventDispatcher->removeEventListener(mListener);
}

void GameStatusLayer::pauseCallback(Ref *sender) {
	Director::getInstance()->pause();
	getScene()->addChild(GamePauseLayer::create());
}

void GameStatusLayer::addGameScore(int score) {
	char str[64];
	mScore += score;
	snprintf(str, sizeof(str), "%d", mScore);
	mScoreLabel->setString(str);
}
