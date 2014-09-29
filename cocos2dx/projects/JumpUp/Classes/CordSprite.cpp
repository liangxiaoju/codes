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
#if 1
bool CordSprite::initWithVertex(Point a, Point b) {
    bool bRet = false;

    do {
        CC_BREAK_IF(!Sprite::initWithFile("cord.png"));

        float distance = a.distance(b);
        auto s = getContentSize();

        const float max_width = 350;
        if (distance > max_width) {
            b = a.lerp(b, max_width/distance);
            distance = max_width;
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
#else
bool CordSprite::initWithVertex(Point a, Point b) {
    setAnchorPoint(Vec2::ANCHOR_MIDDLE);
    setPosition(a.lerp(b, 0.5f));

    this->rotation = -CC_RADIANS_TO_DEGREES(Vec2(b-a).getAngle());
    setRotation(rotation);

    this->a = convertToNodeSpaceAR(a);
    this->b = convertToNodeSpaceAR(b);
    return Node::init();
}

void CordSprite::onEnter() {

    Node::onEnter();

    auto sp = Sprite::create("rope_texture-hd.png");
    auto sf = sp->getSpriteFrame();
    Size sfSize = sp->getContentSize();
    auto distance = a.distance(b);
    int pnum = distance / sfSize.width;
    pnum = pnum > 10 ? 10 : (pnum <= 0 ? 1 : pnum);
    b = a.lerp(b, sfSize.width * pnum / distance);
    distance = a.distance(b);

    Vec2 start = a;
    Vec2 end = b;

    PhysicsBody *A = NULL;
    PhysicsBody *B = NULL;

    Node *first = Node::create();
    first->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
    first->setPosition(start);
    A = PhysicsBody::createCircle(10);
    A->setCategoryBitmask(BITMASK_PHYS_CORD);
    A->setCollisionBitmask(BITMASK_PHYS_HERO);
    A->setContactTestBitmask(BITMASK_PHYS_HERO);
    A->setDynamic(false);
    first->setPhysicsBody(A);
    addChild(first);
    B = A;

    for (int i = 0; i < pnum; i++) {

        auto sp2 = Sprite::createWithSpriteFrame(sf->clone());
        sp2->setPosition(start.lerp(end, (i+(i+1))/2.0f/pnum));
        auto bd2 = PhysicsBody::createBox(sp2->getContentSize());
        bd2->setCategoryBitmask(BITMASK_PHYS_CORD);
        bd2->setCollisionBitmask(BITMASK_PHYS_HERO);
        bd2->setContactTestBitmask(BITMASK_PHYS_HERO);
        sp2->setPhysicsBody(bd2);
        addChild(sp2);

        B = bd2;

        //auto *joint = PhysicsJointSpring::construct(A, B, Point::ZERO, Point::ZERO, 1000, 0.5);
        auto *joint = PhysicsJointLimit::construct(A, B, Point::ZERO, Point::ZERO, 0, 10);
        //auto *joint = PhysicsJointFixed::construct(A, B, Point::ZERO);
        getScene()->getPhysicsWorld()->addJoint(joint);

        A = B;
    }

    Node *last = Node::create();
    last->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
    last->setPosition(end);
    PhysicsBody *lbody = PhysicsBody::createCircle(10);
    lbody->setCategoryBitmask(BITMASK_PHYS_CORD);
    lbody->setCollisionBitmask(BITMASK_PHYS_HERO);
    lbody->setContactTestBitmask(BITMASK_PHYS_HERO);
    lbody->setDynamic(false);
    last->setPhysicsBody(lbody);
    addChild(last);
    //auto *joint = PhysicsJointSpring::construct(B, last->getPhysicsBody(), Point::ZERO, Point::ZERO, 1000, 0.5);
    auto *joint = PhysicsJointLimit::construct(B, last->getPhysicsBody(), Point::ZERO, Point::ZERO, 0, 10);
    //auto *joint = PhysicsJointFixed::construct(B, last->getPhysicsBody(), Point::ZERO);
    getScene()->getPhysicsWorld()->addJoint(joint);
}

void CordSprite::onExit() {
    Node::onExit();
    removeAllChildren();
}
#endif
