#include "GameScene.h"
#include "GameLayer.h"

bool GameScene::init() {
    bool bRet = false;

    do {
        CC_BREAK_IF(!Scene::initWithPhysics());

        getPhysicsWorld()->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);
        getPhysicsWorld()->setSpeed(5.0f);

        auto gameLayer = GameLayer::create();
        addChild(gameLayer);

        bRet = true;
    } while(0);

    return bRet;
}
