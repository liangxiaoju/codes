#include "FightScene.h"
#include "GameLayer.h"
#include "BGLayer.h"

bool FightScene::init()
{
	if (!Scene::init())
		return false;

	auto bgLayer = BGLayer::create();
	addChild(bgLayer);

	auto gameLayer = GameLayer::create();
	addChild(gameLayer);

	return true;
}
