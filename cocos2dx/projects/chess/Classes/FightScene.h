#ifndef __FIGHTSCENE_H__
#define __FIGHTSCENE_H__

#include "cocos2d.h"

USING_NS_CC;

class FightScene : public cocos2d::Scene
{
public:
	virtual bool init() override;
	CREATE_FUNC(FightScene);
};

#endif
