#ifndef __HEADERSPRITE_H__
#define __HEADERSPRITE_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"

USING_NS_CC;
using namespace cocos2d::ui;

class HeaderSprite : public Sprite
{
public:
	enum class Type
	{
		LEFT,
		RIGHT,
	};

	virtual bool initWithType(Type type);
	static HeaderSprite* createWithType(HeaderSprite::Type type);
    void addClickEventListener(const std::function<void(Ref*)>& callback);

	void setNameLine(std::string name);
	void setInfoLine(std::string info);

	void setActive(bool active);

private:
	Type _type;
	Layout *_layout;
    Widget *_head;
};

#endif
