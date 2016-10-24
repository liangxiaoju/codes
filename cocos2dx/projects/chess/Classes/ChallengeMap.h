#ifndef __CHALLENGEMAP_H__
#define __CHALLENGEMAP_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"

USING_NS_CC;
using namespace cocos2d::ui;

class ChallengeMap : public ScrollView
{
public:
    virtual bool init(int total);
    static ChallengeMap *create(int total)
    {
        auto pRet = new (std::nothrow) ChallengeMap();
        if (pRet && pRet->init(total)) {
            pRet->autorelease();
            return pRet;
        } else {
            CC_SAFE_DELETE(pRet);
            return nullptr;
        }
    }

    void setProgress(int progress);
    int getProgress() { return _progress; }
    void addClickEventListener(const std::function<void(int)> &callback);
    void jumpToProgress(int progress);

private:
    std::function<void(int)> _clickEventCallback;
    std::vector<Button*> _flags;
    std::vector<Vec2> _positions;
    Layout *_innerLayout;
    int _total;
    int _progress;
};

#endif
