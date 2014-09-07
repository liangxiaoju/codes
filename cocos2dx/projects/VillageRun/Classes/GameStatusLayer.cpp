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

		mScoreLabel = Label::createWithSystemFont("0", "Marker Felt", 50);
		mScoreLabel->setColor(Color3B::BLACK);
		mScoreLabel->setAnchorPoint(Vec2(1, 1));
		mScoreLabel->setPosition(s.width, s.height);
		addChild(mScoreLabel);

		mScore = 0;
		mListener = EventListenerCustom::create("EVENT_addGameScore", [&](EventCustom* event){
			char *scoreStr = (char *)(event->getUserData());
			int score = 0;
			sscanf(scoreStr, "%d", &score);

			addGameScore(score);
		});
		_eventDispatcher->addEventListenerWithFixedPriority(mListener, 1);

		bRet = true;
	} while(0);

	return bRet;
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
