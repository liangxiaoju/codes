#include "ScoreLayer.h"

ScoreLayer *ScoreLayer::s_scoreLayer = nullptr;

ScoreLayer *ScoreLayer::getInstance() {
    if (s_scoreLayer == nullptr) {
        s_scoreLayer = new ScoreLayer();
        if (s_scoreLayer && s_scoreLayer->init()) {
            return s_scoreLayer;
        } else {
            delete s_scoreLayer;
            return nullptr;
        }
    }
    return s_scoreLayer;
}

bool ScoreLayer::init() {
    bool bRet = false;

    do {
        CC_BREAK_IF(!Layer::init());

        Size winSize = Director::getInstance()->getWinSize();

        mLabel = Label::createWithBMFont("fonts/score.fnt", "0");
        CC_BREAK_IF(!mLabel);
        mLabel->setAnchorPoint(Vec2(0, 1));
        mLabel->setPosition(0+5, winSize.height-5);
        addChild(mLabel);

        loadHistory();

        bRet = true;
    } while (0);

    return bRet;
}

void ScoreLayer::onExit() {
    saveHistory();
    Layer::onExit();
}

void ScoreLayer::addScore(int score) {
    char str[16];

    mScore += score;
    snprintf(str, sizeof(str), "%d", mScore);
    mLabel->setString(str);
}

int ScoreLayer::getCurrentScore() {
    return mScore;
}

void ScoreLayer::clear() {
    saveHistory();
    mScore = 0;
    mLabel->setString("0");
}

int ScoreLayer::getHighestScore() {
    return mHighestScore > mScore ? mHighestScore : mScore;
}

void ScoreLayer::saveHistory() {
    mHighestScore = mHighestScore > mScore ? mHighestScore : mScore;
    CCLog("saveHistory() highest:%d current:%d\n", mHighestScore, mScore);
    UserDefault::getInstance()->setIntegerForKey("HighestScore", mHighestScore);
    UserDefault::getInstance()->flush();
}

void ScoreLayer::loadHistory() {
    mHighestScore = UserDefault::getInstance()->getIntegerForKey("HighestScore", 0);
    CCLog("loadHistory() highest:%d\n", mHighestScore);
}

