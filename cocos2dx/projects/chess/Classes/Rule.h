#ifndef __RULE_H__
#define __RULE_H__

#include "cocos2d.h"
#include "PositionStruct.h"

USING_NS_CC;

class Rule : public Ref
{
public:
	bool init();
	static Rule *getInstance();

	int check(std::string fen);
	bool isMate(std::string fen);
    bool isChecked(std::string fen);
    bool isCaptured(std::string fen);

private:
	static Rule *s_rule;

	SimplePosition::PositionStruct _pos;
};

#endif
