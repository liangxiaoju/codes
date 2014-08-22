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
		CC_BREAK_IF(!TMXTiledMap::initWithTMXFile("maps/basicLevels/"+tmxFile));

        auto layer1 = getLayer("Tile");
        layer1->setVisible(false);
        auto layer2 = getLayer("Collision");
        layer2->setVisible(false);
        auto group = getObjectGroup("Main");
        auto& objs = group->getObjects();

        for (auto& obj : objs) {

            ValueMap& dict = obj.asValueMap();

            std::string name = dict["name"].asString();
            std::string type = dict["type"].asString();
            float x = dict["x"].asFloat();
            float y = dict["y"].asFloat();
            float width = dict["width"].asFloat();
            float height = dict["height"].asFloat();

            if (name.compare("rect") == 0) {
                Node *node = Node::create();
                node->setAnchorPoint(Vec2(0.5, 0.5));
                node->setPosition(x+width/2, y+height/2);
                Size size = Size(width, height);
                node->setContentSize(size);
                PhysicsBody *body = PhysicsBody::createEdgeBox(size);
                body->setTag(TAG_BARRIER_PHYS_BODY);
		        body->setContactTestBitmask(0xFFFFFFFF);
                node->setPhysicsBody(body);
                addChild(node);
            } else {
                Sprite *sprite = Sprite::create("ui/spriteSheet/"+name+".png");
                sprite->setPosition(x+width/2, y+height/2);
                addChild(sprite);
            }
        }

		bRet = true;
	} while(0);

	return bRet;
}
