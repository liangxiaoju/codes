#include "AdController.h"
#include "jni/JniHelper.h"

AdController *AdController::s_adController = NULL;

AdController *AdController::getInstance() {
    if (s_adController == NULL) {
        s_adController = new AdController();
    }
    return s_adController;
}

void AdController::setVisible(bool visible) {
    JniMethodInfo t;
    if (JniHelper::getStaticMethodInfo(t, "org/cocos2dx/cpp/AppActivity", "showAd", "(Z)V")) {
        t.env->CallStaticVoidMethod(t.classID, t.methodID, visible);
        t.env->DeleteLocalRef(t.classID);
    }
    CCLog("AdController::setVisible(%d)\n", visible);
}
