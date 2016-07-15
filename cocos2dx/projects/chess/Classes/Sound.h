#ifndef __SOUND_H__
#define __SOUND_H__

#include "cocos2d.h"
#include "SimpleAudioEngine.h"
#include <unordered_map>

USING_NS_CC;
using namespace CocosDenshion;

class Sound
{
public:
    static Sound *getInstance();
    bool init();

    void playBackgroundMusic();
    void stopBackgroundMusic();
    void playEffect(std::string effect);

private:
    static Sound *s_sound;
    SimpleAudioEngine *_audio;
    std::unordered_map<std::string, std::string> _pathMap;
};

#endif
