#include "JavaHelper.h"

JavaHelper *JavaHelper::s_javaHelper = NULL;

JavaHelper *JavaHelper::getInstance() {
    if (s_javaHelper == NULL) {
        s_javaHelper = new JavaHelper();
    }
    return s_javaHelper;
}

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include "jni/JniHelper.h"
void JavaHelper::showAds() {
    JniMethodInfo t;
    if (JniHelper::getStaticMethodInfo(t, "org/cocos2dx/cpp/AppActivity", "showAds", "()V")) {
        t.env->CallStaticVoidMethod(t.classID, t.methodID);
        t.env->DeleteLocalRef(t.classID);
    }
}
void JavaHelper::hideAds() {
    JniMethodInfo t;
    if (JniHelper::getStaticMethodInfo(t, "org/cocos2dx/cpp/AppActivity", "hideAds", "()V")) {
        t.env->CallStaticVoidMethod(t.classID, t.methodID);
        t.env->DeleteLocalRef(t.classID);
    }
}
void JavaHelper::showAchievements() {
    JniMethodInfo t;
    if (JniHelper::getStaticMethodInfo(t, "org/cocos2dx/cpp/AppActivity", "showAchievements", "()V")) {
        t.env->CallStaticVoidMethod(t.classID, t.methodID);
        t.env->DeleteLocalRef(t.classID);
    }
}
void JavaHelper::showLeaderboards() {
    JniMethodInfo t;
    if (JniHelper::getStaticMethodInfo(t, "org/cocos2dx/cpp/AppActivity", "showLeaderboards", "()V")) {
        t.env->CallStaticVoidMethod(t.classID, t.methodID);
        t.env->DeleteLocalRef(t.classID);
    }
}
void JavaHelper::storeLeaderboards(int score) {
    JniMethodInfo t;
    if (JniHelper::getStaticMethodInfo(t, "org/cocos2dx/cpp/AppActivity", "storeLeaderboards", "(I)V")) {
        t.env->CallStaticVoidMethod(t.classID, t.methodID, score);
        t.env->DeleteLocalRef(t.classID);
    }
}
#else
void JavaHelper::showAd() {}
void JavaHelper::hideAd() {}
void JavaHelper::showAchievements() {}
void JavaHelper::showLeaderboard() {}
void JavaHelper::storeLeaderboards(int score) {}
#endif

