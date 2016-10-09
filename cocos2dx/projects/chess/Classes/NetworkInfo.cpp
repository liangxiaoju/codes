#include "NetworkInfo.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include "jni/JniHelper.h"
#endif

static const std::string helperClassName = "org/cocos2dx/cpp/AppActivity";

NetworkInfo *NetworkInfo::s_networkInfo = NULL;

NetworkInfo *NetworkInfo::getInstance()
{
    if (s_networkInfo == NULL)
        s_networkInfo = new NetworkInfo();
    return s_networkInfo;
}

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
bool NetworkInfo::isWifiConnected()
{
    return JniHelper::callStaticBooleanMethod(helperClassName, "isWifiConnected");
}
#else
bool NetworkInfo::isWifiConnected()
{
    return true;
}
#endif
