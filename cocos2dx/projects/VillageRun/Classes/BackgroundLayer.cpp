#include "BackgroundLayer.h"
#include "BarrierMap.h"
#include "Constant.h"

int basicLevelValue[][10] = {
    { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 },
    { 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 },
    { 21, 22, 23, 24, 25, 26, 27, 28, 29, 30 },
    { 31, 32, 33, 34, 35, 36, 37, 38, 39, 40 },
    { 41, 42, 43, 44, 45, 46, 47, 48, 49, 50 },
};

int proLevelValue[][30] = {
};

BackgroundLayer::BackgroundLayer() {

    memcpy(levelValue, basicLevelValue, sizeof(levelValue));
    valueIndex = 0;
    levelIndex = 0;
}

bool BackgroundLayer::init() {
	bool bRet = false;

	do {
		CC_BREAK_IF(!Layer::init());

		Size wsize = Director::getInstance()->getVisibleSize();

		Sprite *city = Sprite::create("ui/spriteSheet/cityBG.png");
		Size citySize = city->getContentSize();
		city->setPosition(wsize.width/2, wsize.height-citySize.height/2+30);
		addChild(city);
        Point cityPoint = city->getPosition();

		Sprite *ground1 = Sprite::create("ui/spriteSheet/ground.png");
		Sprite *ground2 = Sprite::create("ui/spriteSheet/ground.png");
		Size gdsize = ground1->getContentSize();
		ground1->setPosition(gdsize.width/2, cityPoint.y-citySize.height/2-gdsize.height/2);
		ground2->setPosition(gdsize.width*1.5, cityPoint.y-citySize.height/2-gdsize.height/2);

		PhysicsBody *body1 = PhysicsBody::createBox(gdsize, PhysicsMaterial(1,0,0.5));
		body1->setContactTestBitmask(0xFFFFFFFF);
        body1->setDynamic(false);
		body1->setRotationEnable(false);
        body1->setTag(TAG_GROUND1_PHYS_BODY);
		ground1->setPhysicsBody(body1);

		PhysicsBody *body2 = PhysicsBody::createBox(gdsize, PhysicsMaterial(1,0,0.5));
		body2->setContactTestBitmask(0xFFFFFFFF);
        body2->setDynamic(false);
		body2->setRotationEnable(false);
        body2->setTag(TAG_GROUND2_PHYS_BODY);
		ground2->setPhysicsBody(body2);

		addChild(ground1, 0, "ground1");
		addChild(ground2, 0, "ground2");
        Point gdPoint = ground1->getPosition();

		startMove();

		bRet = true;
	} while(0);

	return bRet;
}

void BackgroundLayer::startMove() {
    groundMove();
    mapMove();
}

void BackgroundLayer::stopMove() {
}

void BackgroundLayer::groundMove() {
    Sprite *ground1 = (Sprite *)getChildByName("ground1");
    Sprite *ground2 = (Sprite *)getChildByName("ground2");
    Point pos = ground1->getPosition();
    Size gdsize = ground1->getContentSize();
    MoveBy *move = MoveBy::create(3, Vec2(-gdsize.width, 0));
    Place *place = Place::create(Vec2(gdsize.width*1.5, pos.y));
    Sequence *seq1 = Sequence::create(move, place, move->clone(), nullptr);
    Sequence *seq2 = Sequence::create(move->clone(), move->clone(), place->clone(), nullptr);
    ground1->runAction(RepeatForever::create(seq1));
    ground2->runAction(RepeatForever::create(seq2));

}

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

void BackgroundLayer::mapMove() {
    char name[32];

    snprintf(name, sizeof(name), "basicLevel_%d.tmx", levelValue[levelIndex][valueIndex]);

    valueIndex = (++valueIndex) % ARRAY_SIZE(levelValue[levelIndex]);
    if (valueIndex == 0)
        levelIndex = (++levelIndex) % ARRAY_SIZE(levelValue);

    auto map = BarrierMap::create(name);
    Size s = map->getContentSize();
    map->setPosition(s.width, 0);
    addChild(map, 0, "map");
    MoveBy *move = MoveBy::create(3, Vec2(-s.width, 0));
    CallFunc *callback = CallFunc::create([&]() { mapMove(); });
    Sequence *seq = Sequence::create(
            move,
            Spawn::create(callback, move->clone(), nullptr),
            RemoveSelf::create(), nullptr);
    map->runAction(seq);
}
