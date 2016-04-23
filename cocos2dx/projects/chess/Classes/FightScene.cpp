#include "FightScene.h"
#include "GameLayer.h"
#include "BGLayer.h"
#include "MainMenuScene.h"
#include "FightControl.h"

bool FightScene::init(GameLayer::Mode mode, Piece::Side side, int level, std::string fen)
{
	if (!Scene::init())
		return false;

	auto visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	auto bgLayer = BGLayer::create();
	addChild(bgLayer);

	auto gameLayer = GameLayer::create(mode, side, level, fen);
	addChild(gameLayer);

	auto fightControl = FightControl::create();
	addChild(fightControl);
	fightControl->setGameLayer(gameLayer);

	return true;
}
