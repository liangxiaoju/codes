#ifndef __JAVAHELPER_H__
#define __JAVAHELPER_H__

#include "cocos2d.h"

USING_NS_CC;

class JavaHelper {
public:
    static JavaHelper *getInstance();

    void showAds();
    void hideAds();
    void showAchievements();
    void showLeaderboards();
    void storeLeaderboards(int score);

private:
    static JavaHelper *s_javaHelper;
};

#endif
