#ifndef __SCORELAYER_H__
#define __SCORELAYER_H__

#include "cocos2d.h"

USING_NS_CC;

class ScoreLayer : public Layer {
public:
    virtual bool init();
    void addScore(int score);
    int getCurrentScore();
    int getHighestScore();
    static ScoreLayer *getInstance();
    void loadHistory();
    void saveHistory();
    void onExit() override;
    void clear();

private:
    Label *mLabel;
    int mScore;
    static ScoreLayer *s_scoreLayer;
    int mHighestScore;
};

#endif
