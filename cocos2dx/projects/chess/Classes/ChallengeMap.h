#ifndef __CHALLENGEMAP_H__
#define __CHALLENGEMAP_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"

USING_NS_CC;
using namespace cocos2d::ui;

class ChallengeMap : public Layout
{
public:
    virtual bool init(int total, int progress);
    static ChallengeMap *create(int total, int progress)
    {
        auto pRet = new (std::nothrow) ChallengeMap();
        if (pRet && pRet->init(total, progress)) {
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

private:
    std::function<void(int)> _clickEventCallback;
    std::vector<Button*> _passFlags;
    Button *_currentFlag;
    std::vector<Vec2> _positions;
    int _total;
    int _progress;
};

#endif
