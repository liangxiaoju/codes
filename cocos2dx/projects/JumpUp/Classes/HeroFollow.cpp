#include "HeroFollow.h"

HeroFollow* HeroFollow::create(Node *followedNode, const Rect& rect/* = Rect::ZERO*/)
{
    HeroFollow *follow = new HeroFollow();
    if (follow && follow->initWithTarget(followedNode, rect))
    {
        follow->autorelease();
        return follow;
    }
    CC_SAFE_DELETE(follow);
    return nullptr;
}

void HeroFollow::step(float dt) {
    CC_UNUSED_PARAM(dt);

    Point p = _followedNode->getPosition();
    p = _target->convertToWorldSpace(p);

    Size vsize = Director::getInstance()->getVisibleSize();

    Vec2 v = _followedNode->getPhysicsBody()->getVelocity();
    if (v.y <= 0) {
        return;
    }

    if (p.y > vsize.height/2) {

        if(_boundarySet)
        {
            if(_boundaryFullyCovered)
                return;

            Vec2 tempPos = _halfScreenSize - _followedNode->getPosition();

            _target->setPositionY(clampf(tempPos.y, _bottomBoundary, _topBoundary));
        }
        else
        {
            _target->setPositionY(_halfScreenSize.y - _followedNode->getPositionY());
        }
    }

}
