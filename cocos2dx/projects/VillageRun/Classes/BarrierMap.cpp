#include "BarrierMap.h"
#include "Constant.h"

BarrierMap* BarrierMap::create(const std::string& tmxFile) {
    BarrierMap* ret = new BarrierMap();
    if (ret->initWithTMXFile(tmxFile)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool BarrierMap::initWithTMXFile(const std::string& tmxFile) {
	bool bRet = false;

	do {
		setContentSize(Director::getInstance()->getVisibleSize());

		TMXMapInfo *mapInfo = TMXMapInfo::create(tmxFile);
		if (!mapInfo)
			break;

        cocos2d::Vector<cocos2d::TMXLayerInfo *> info1;
        cocos2d::Vector<cocos2d::TMXTilesetInfo *> info2;

		/* do not use layers and tilesets */
		info1.clear();
		info2.clear();
		mapInfo->setLayers(info1);
		mapInfo->setTilesets(info2);

		buildWithMapInfo(mapInfo);

        auto group = getObjectGroup("Main");
        auto& objs = group->getObjects();

		std::vector<Rect> rects;

		for (auto& obj : objs) {
            ValueMap& dict = obj.asValueMap();
            std::string name = dict["name"].asString();

			if (name == "rect") {
				float x = dict["x"].asFloat();
				float y = dict["y"].asFloat();
				float width = dict["width"].asFloat();
				float height = dict["height"].asFloat();

				Rect rect(x, y, width, height);
				rects.push_back(rect);
			}
		}

		for (auto& obj : objs) {
            ValueMap& dict = obj.asValueMap();
            std::string name = dict["name"].asString();

			if (name != "rect") {
				float x = dict["x"].asFloat();
				float y = dict["y"].asFloat();
				float width = dict["width"].asFloat();
				float height = dict["height"].asFloat();

				Rect spriteRect(x, y, width, height);

                PhysicsBody *body = PhysicsBody::create();
				for (auto rect : rects) {
					if (spriteRect.intersectsRect(rect)) {
						body->addShape(PhysicsShapeBox::create(rect.size));
					}
				}
				body->setDynamic(false);
				body->setTag(TAG_BARRIER_PHYS_BODY);
				body->setCategoryBitmask(1<<1);
				body->setContactTestBitmask(1<<2);

                Sprite *sprite = Sprite::createWithSpriteFrameName(name+".png");
                sprite->setPosition(x+width/2, y+height/2);
				sprite->setPhysicsBody(body);
                addChild(sprite);
			}
		}

		bRet = true;
	} while(0);

	return bRet;
}

