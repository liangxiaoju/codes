#ifndef __NETWORKINFO_H__
#define __NETWORKINFO_H__

#include "cocos2d.h"

USING_NS_CC;

class NetworkInfo {
public:
    static NetworkInfo *getInstance();
    bool isWifiConnected();

private:
    static NetworkInfo *s_networkInfo;
};

#endif
