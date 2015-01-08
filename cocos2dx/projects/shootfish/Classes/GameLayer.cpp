#include "GameLayer.h"
#include "UFOSprite.h"
#include "MenuScene.h"

using namespace CocosDenshion;

GameLayer *GameLayer::s_gameLayer = nullptr;

GameLayer *GameLayer::getInstance() {
    if (!s_gameLayer) {
        s_gameLayer = new GameLayer();
        if (s_gameLayer) {
            s_gameLayer->init();
        }
    }
    return s_gameLayer;
}

bool GameLayer::init() {
    bool bRet = false;

    do {
        CC_BREAK_IF(!Layer::init());

        restart();

        setKeyboardEnabled(true);

        bRet = true;
    } while(0);

    return bRet;
}

void GameLayer::restart() {
    stopAllActions();
    unscheduleAllSelectors();
    removeAllChildren();

    Size s = Director::getInstance()->getVisibleSize();

    mPlaneSprite = PlaneSprite::create();
    addChild(mPlaneSprite, 0, "plane");

    schedule(schedule_selector(GameLayer::showEnemy), 0.8, kRepeatForever, 1);
    schedule(schedule_selector(GameLayer::showUFO), 40, kRepeatForever, 15);
    scheduleUpdate();

    mBomb = 0;

    Sprite *s1 = Sprite::createWithSpriteFrameName("bomb.png");
    Sprite *s1p = Sprite::createWithSpriteFrameName("bomb.png");
    s1p->setColor(Color3B::GRAY);
    mBombSprite = MenuItemSprite::create(
            s1, s1p, [&](Ref *sender) { GameLayer::useBomb(); });
    mBombSprite->setAnchorPoint(Vec2(0, 0));
    mBombSprite->setPosition(0, 0);

    Sprite *s2 = Sprite::createWithSpriteFrameName("game_pause_nor.png");
    Sprite *s2p = Sprite::createWithSpriteFrameName("game_pause_pressed.png");
    mPauseSprite = MenuItemSprite::create(
            s2, s2p, [&](Ref *sender) { GameLayer::gamePause(); });
    mPauseSprite->setAnchorPoint(Vec2(1, 1));
    mPauseSprite->setPosition(s.width, s.height);

    auto menu = Menu::create(mBombSprite, mPauseSprite, nullptr);
    menu->setPosition(Vec2::ZERO);
    addChild(menu);

    char buf[8];
    snprintf(buf, sizeof(buf), "X%d", mBomb);
    mBombLabel = Label::createWithBMFont("fonts/score.fnt", buf);
    mBombLabel->setAnchorPoint(Vec2(0, 0));
    Size bombSize = mBombSprite->getContentSize();
    mBombLabel->setPosition(bombSize.width+5, 15);
    addChild(mBombLabel);

    updateBomb();
}

void GameLayer::useBomb() {
    if (mBomb > 0) {
        mBomb--;
        bombAllEnemys();
        updateBomb();
        SimpleAudioEngine::getInstance()->playEffect("sound/use_bomb.mp3", false);
    }
}

void GameLayer::updateBomb() {
    char buf[8];
    snprintf(buf, sizeof(buf), "X%d", mBomb);
    mBombLabel->setString(buf);

    if (mBomb > 0) {
        mBombSprite->setVisible(true);
        mBombLabel->setVisible(true);
    } else {
        mBombSprite->setVisible(false);
        mBombLabel->setVisible(false);
    }
}

void GameLayer::showEnemy(float dt) {
    EnemySprite *enemy;
    int score = ScoreLayer::getInstance()->getCurrentScore();

    enemy = EnemySprite::create(1.0 + score/500000.0 * 0.5);

    addChild(enemy, 0, "enemy");
}

void GameLayer::showUFO(float dt) {
    float random = CCRANDOM_0_1();
    int type = random < 0.3 ? 1 : (random < 0.6 ? 2 : 3);
    UFOSprite *ufo = UFOSprite::create(type);
    addChild(ufo, 0, "ufo");
}

void GameLayer::bombAllEnemys() {
    std::vector<Node*> enemyChilds = utils::findChildren(*this, "enemy");
    for (auto& enemyChild : enemyChilds) {
        if (((EnemySprite *)enemyChild)->isAlive()) {
            ((EnemySprite *)enemyChild)->blowUp();
        }
    }
}

void GameLayer::update(float fDelta) {
    Rect planeBox;
    Rect enemyBox;
    Rect bulletBox;
    Rect UFOBox;

    std::vector<Node*> enemyChilds = utils::findChildren(*this, "enemy");
    std::vector<Node*> bulletChilds = utils::findChildren(*this, "bullet");
    std::vector<Node*> UFOChilds = utils::findChildren(*this, "ufo");

    if (mPlaneSprite) {
        planeBox = mPlaneSprite->getBoundingBox();
        planeBox.origin += Vec2(planeBox.size/3/2);
        planeBox.size = planeBox.size * (2.0/3);

        for (auto& UFOChild : UFOChilds) {
            UFOBox = UFOChild->getBoundingBox();

            if (planeBox.intersectsRect(UFOBox)) {
                int type = ((UFOSprite *)UFOChild)->getType();
                if (type == 1) {
                    mPlaneSprite->superPower();
                } else if (type == 2) {
                    mBomb++;
                    updateBomb();
                    SimpleAudioEngine::getInstance()->playEffect("sound/get_bomb.mp3", false);
                } else if (type == 3) {
                    mPlaneSprite->blowUp();
                    mPlaneSprite = NULL;
                    gameOver();
                }
                removeChild(UFOChild);
            }
        }

        for (auto& enemyChild : enemyChilds) {

            if (!((EnemySprite *)enemyChild)->isAlive())
                continue;

            enemyBox = enemyChild->getBoundingBox();
            enemyBox.origin += Vec2(enemyBox.size/3/2);
            enemyBox.size = enemyBox.size * (2.0/3);

            for (auto& bulletChild : bulletChilds) {
                bulletBox = bulletChild->getBoundingBox();

                if (enemyBox.intersectsRect(bulletBox)) {
                    ((BulletSprite *)bulletChild)->blowUp();
                    ((EnemySprite *)enemyChild)->hit();
                    break;
                }
            }

            if (!((EnemySprite *)enemyChild)->isAlive()) {
                ((EnemySprite *)enemyChild)->blowUp();
                continue;
            }

            if (planeBox.intersectsRect(enemyBox)) {
                mPlaneSprite->blowUp();
                mPlaneSprite = NULL;
                ((EnemySprite *)enemyChild)->blowUp();

                gameOver();

                break;
            }
        }
    }
}

void GameLayer::gameOver() {
    DelayTime *delay = DelayTime::create(1.0);
    CallFunc *callback = CallFunc::create([]() {
        auto tran = TransitionMoveInT::create(0, GameOverScene::create());
        Director::getInstance()->replaceScene(tran);
    });
    Sequence *seq = Sequence::create(delay, callback, nullptr);
    runAction(seq);
}

void GameLayer::gamePause() {
    auto callback = [&]() {
        Size s = Director::getInstance()->getVisibleSize();
        auto texture = RenderTexture::create(s.width, s.height);
        texture->begin();
        getScene()->visit();
        texture->end();

        Director::getInstance()->pushScene(MenuScene::create(texture));
    };

    getScheduler()->performFunctionInCocosThread(callback);
}

void GameLayer::onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event) {
    switch (keyCode) {
    case EventKeyboard::KeyCode::KEY_BACK:
        gamePause();
        break;
    default:
        break;
    }
}

