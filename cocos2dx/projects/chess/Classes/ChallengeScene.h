#ifndef __CHALLENGESCENE_H__
#define __CHALLENGESCENE_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "EndGameData.h"

USING_NS_CC;
using namespace cocos2d::ui;

class ChallengeScene : public Scene
{
public:
	virtual bool init(EndGameData::EndGameItem item);
	static ChallengeScene *create(EndGameData::EndGameItem item)
	{
		ChallengeScene *pRet = new(std::nothrow) ChallengeScene();
		if (pRet && pRet->init(item)) {
			pRet->autorelease();
			return pRet;
		} else {
			delete pRet;
			pRet = nullptr;
			return nullptr;
		}
	}

private:
	EndGameData::EndGameItem _item;
};

class ChallengeMenuL1 : public Scene
{
public:
	virtual bool init();
	CREATE_FUNC(ChallengeMenuL1);

private:
	std::vector<EndGameData::EndGameClass> _endGameClass;
	Layout *_headview;
	ListView *_listview;
	Layout *_default_model;
};

class ChallengeMenuL2 : public Scene
{
public:
	virtual bool init(EndGameData::EndGameClass cls);

	static ChallengeMenuL2 *create(EndGameData::EndGameClass cls)
	{
		ChallengeMenuL2 *pRet = new(std::nothrow) ChallengeMenuL2();
		if (pRet && pRet->init(cls)) {
			pRet->autorelease();
			return pRet;
		} else {
			delete pRet;
			pRet = nullptr;
			return nullptr;
		}
	}

private:
	EndGameData::EndGameClass _endGameClass;
	std::vector<EndGameData::EndGameItem> _endGameItems;
	Layout *_headview;
	ListView *_listview;
	Layout *_default_model;
};

class ChallengeMapScene : public Scene
{
public:
    virtual bool init();
    CREATE_FUNC(ChallengeMapScene);
};

#endif
