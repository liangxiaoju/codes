#include "CordSprite.h"
#include "Constant.h"

CordSprite *CordSprite::createWithVertex(Point a, Point b) {
    CordSprite *cord = new CordSprite();
    if (cord && cord->initWithVertex(a, b)) {
        cord->autorelease();
        return cord;
    }
    CC_SAFE_DELETE(cord);
    return nullptr;
}

bool CordSprite::initWithVertex(Point a, Point b) {
    bool bRet = false;

    do {
        CC_BREAK_IF(!Sprite::initWithFile("cord.png"));

        float distance = a.distance(b);
        auto s = getContentSize();

        if (distance > s.width) {
            b = a.lerp(b, s.width/distance);
            distance = s.width;
        }

        setTextureRect(Rect(0, 0, distance, s.height));
        s = getContentSize();

        float rotation = -CC_RADIANS_TO_DEGREES(Vec2(b-a).getAngle());

        setRotation(rotation);
        setAnchorPoint(Vec2::ANCHOR_MIDDLE);
        setPosition(a.lerp(b, 0.5f));
#if 1
        auto body = PhysicsBody::createEdgeSegment(
                Vec2(-s.width/2, s.height/2), s/2,
                PhysicsMaterial(10, 0.5, 0.5), 10);
#else
        std::vector<Vec2> points;
        int pnum = 5;
        Vec2 start = Vec2(-s.width/2, s.height/2);
        Vec2 end = s/2;
        for (int i = 0; i <= pnum; i++) {
            points.push_back(start.lerp(end, i/pnum));
        }

        auto body = PhysicsBody::createEdgeChain(&points[0], pnum+1,
                PhysicsMaterial(10, 0.5, 0.5), 5);
#endif
        body->setCategoryBitmask(BITMASK_PHYS_CORD);
        body->setContactTestBitmask(BITMASK_PHYS_HERO);
        setPhysicsBody(body);

        bRet = true;
    } while (0);

    return bRet;
}

