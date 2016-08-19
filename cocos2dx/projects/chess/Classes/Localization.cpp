#include "Localization.h"

Localization *Localization::s_localization = nullptr;

Localization *Localization::getInstance()
{
    if (s_localization == nullptr) {
        s_localization = new (std::nothrow) Localization();
        if (s_localization) {
            if (!s_localization->init())
                CC_SAFE_DELETE(s_localization);
        }
    }
    return s_localization;
}

bool Localization::init()
{
    _localizationJson = cocostudio::JsonLocalizationManager::getInstance();

    LanguageType currentLanguageType =
        Application::getInstance()->getCurrentLanguage();

    switch (currentLanguageType)
    {
    case LanguageType::CHINESE:
        _localizationJson->initLanguageData("language/zh-CN.lang.json");
        break;
    case LanguageType::ENGLISH:
    default:
        _localizationJson->initLanguageData("language/en-US.lang.json");
        break;
    }

    return true;
}

std::string Localization::getLocalizationString(std::string key)
{
    return _localizationJson->getLocalizationString(key);
}
