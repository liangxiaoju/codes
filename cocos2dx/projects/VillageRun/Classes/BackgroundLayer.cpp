#include "BackgroundLayer.h"

bool BackgroundLayer::init() {
	bool bRet = false;

	do {
		CC_BREAK_IF(!Layer::init());

		Size wsize = Director::getInstance()->getWinSize();

		Sprite *bg = Sprite::create("ui/spriteSheet/cityBG.png");
		bg->setAnchorPoint(Vec2(0, 1));
		bg->setPosition(0, wsize.height);
		addChild(bg);
		Size bgsize = bg->getContentSize();

		Sprite *ground1 = Sprite::create("ui/spriteSheet/ground.png");
		Sprite *ground2 = Sprite::create("ui/spriteSheet/ground.png");
		Size gdsize = ground1->getContentSize();
		ground1->setPosition(gdsize.width/2, wsize.height-bgsize.height-gdsize.height/2);
		ground2->setPosition(gdsize.width*1.5, wsize.height-bgsize.height-gdsize.height/2);

		PhysicsBody *body1 = PhysicsBody::createEdgeBox(gdsize, PHYSICSSHAPE_MATERIAL_DEFAULT, 3);
		body1->setContactTestBitmask(0xFFFFFFFF);
		body1->setLinearDamping(0);
		body1->setRotationEnable(false);
		ground1->setPhysicsBody(body1);

		PhysicsBody *body2 = PhysicsBody::createEdgeBox(gdsize, PHYSICSSHAPE_MATERIAL_DEFAULT, 3);
		body2->setContactTestBitmask(0xFFFFFFFF);
		body2->setLinearDamping(0);
		body2->setRotationEnable(false);
		ground2->setPhysicsBody(body2);

		addChild(ground1, 0, "ground1");
		addChild(ground2, 0, "ground2");
/*
		Node *ground = Node::create();
		ground->setContentSize(gdsize);
		ground->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
		ground->setPosition(wsize.width/2, wsize.height-bgsize.height-gdsize.height/2);
		PhysicsBody *body = PhysicsBody::createBox(gdsize, PHYSICSSHAPE_MATERIAL_DEFAULT);
		body->setDynamic(false);
		body->setContactTestBitmask(0xFFFFFFFF);
		ground->setPhysicsBody(body);
		addChild(ground, 0, "groundNode");
*/
		startMove();

		CCLog("%f\n", ground1->getPositionY());

		bRet = true;
	} while(0);

	return bRet;
}

void BackgroundLayer::startMove() {
	schedule(schedule_selector(BackgroundLayer::groundMove), 0.01f);
}

void BackgroundLayer::stopMove() {
	unschedule(schedule_selector(BackgroundLayer::groundMove));
}

void BackgroundLayer::groundMove(float dt) {
	Sprite *ground1 = (Sprite *)getChildByName("ground1");
	Sprite *ground2 = (Sprite *)getChildByName("ground2");
	Size gdsize = ground1->getContentSize();

	Point pos1 = ground1->getPosition();
	ground1->setPosition(pos1.x-4, pos1.y);
	pos1 = ground1->getPosition();
	ground2->setPosition(pos1.x + gdsize.width, pos1.y);

	if (ground2->getPositionX() <= gdsize.width/2) {
		ground1->setPosition(ground2->getPosition());
	}
}
