#ifndef __LOCALIZATION_H__
#define __LOCALIZATION_H__

#include "cocos2d.h"
#include "editor-support/cocostudio/LocalizationManager.h"

USING_NS_CC;

class Localization
{
public:
    bool init();
    static Localization *getInstance();

    std::string getLocalizationString(std::string key);

private:
    static Localization *s_localization;
    cocostudio::ILocalizationManager* _localizationJson;
};

#define TR(x) Localization::getInstance()->getLocalizationString(x)

#endif
