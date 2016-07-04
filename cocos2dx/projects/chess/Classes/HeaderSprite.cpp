#include "HeaderSprite.h"

HeaderSprite* HeaderSprite::createWithType(HeaderSprite::Type type)
{
    HeaderSprite *sprite = new (std::nothrow) HeaderSprite();
    if (sprite && sprite->initWithType(type))
    {
        sprite->autorelease();
        return sprite;
    }
    CC_SAFE_DELETE(sprite);
    return nullptr;
}

bool HeaderSprite::initWithType(Type type)
{
	if (!Sprite::init())
		return false;

	auto size = Size(400, 150);
	setContentSize(size);
	auto rbox = RelativeBox::create(size);
	auto line = ImageView::create("head/name_line.png");
	auto mask = ImageView::create("head/commhead_mask.png");
	auto head = Button::create("head/game_Default_head.png");
	rbox->addChild(line);
	rbox->addChild(mask);
	rbox->addChild(head, 0, "head");

	RelativeLayoutParameter* rp_center = RelativeLayoutParameter::create();
	rp_center->setAlign(RelativeLayoutParameter::RelativeAlign::CENTER_IN_PARENT);
	rp_center->setRelativeName("line");
	line->setLayoutParameter(rp_center);

	if (type == Type::LEFT) {
		rbox->setBackGroundImage("head/bg_r.png");
		RelativeLayoutParameter* rp_l = RelativeLayoutParameter::create();
		rp_l->setAlign(RelativeLayoutParameter::RelativeAlign::PARENT_LEFT_CENTER_VERTICAL);
		mask->setLayoutParameter(rp_l);
		head->setLayoutParameter(rp_l->clone());
	} else {
		rbox->setBackGroundImage("head/bg_l.png");
		RelativeLayoutParameter* rp_r = RelativeLayoutParameter::create();
		rp_r->setAlign(RelativeLayoutParameter::RelativeAlign::PARENT_RIGHT_CENTER_VERTICAL);
		mask->setLayoutParameter(rp_r);
		head->setLayoutParameter(rp_r->clone());
	}

	addChild(rbox);
	_layout = rbox;

	return true;
}

void HeaderSprite::setNameLine(std::string name)
{
	auto text = Text::create(name, "", 35);
	text->setTextColor(Color4B::YELLOW);

	RelativeLayoutParameter* rp_top = RelativeLayoutParameter::create();
	rp_top->setRelativeToWidgetName("line");
	rp_top->setAlign(RelativeLayoutParameter::RelativeAlign::LOCATION_ABOVE_CENTER);
	text->setLayoutParameter(rp_top);
	if (_layout->getChildByName("nameLine"))
		_layout->removeChildByName("nameLine");
	_layout->addChild(text, 0, "nameLine");
}

void HeaderSprite::setInfoLine(std::string info)
{
	auto text = Text::create(info, "", 25);
	text->setTextColor(Color4B::YELLOW);

	RelativeLayoutParameter* rp_bottom = RelativeLayoutParameter::create();
	rp_bottom->setRelativeToWidgetName("line");
	rp_bottom->setAlign(RelativeLayoutParameter::RelativeAlign::LOCATION_BELOW_CENTER);
	text->setLayoutParameter(rp_bottom);
	if (_layout->getChildByName("infoLine"))
		_layout->removeChildByName("infoLine");
	_layout->addChild(text, 0, "infoLine");
}

void HeaderSprite::setActive(bool active)
{
	Button *head = (Button*)_layout->getChildByName("head");
	Size size = head->getContentSize();

	if (active) {
		RotateBy *rotateBy = RotateBy::create(0.1, -15);
		RepeatForever *repeat = RepeatForever::create(rotateBy);
		auto sprite = Sprite::create("head/load_outter.png");

		sprite->runAction(repeat);
		sprite->setPosition(size.width/2, size.height/2);
		head->addChild(sprite, 0, "active");
	} else {
		if (head->getChildByName("active"))
			head->removeChildByName("active");
	}
}
