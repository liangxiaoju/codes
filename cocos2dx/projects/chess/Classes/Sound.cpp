#include "Sound.h"

#define SOUND_DIR "sound/"

Sound *Sound::s_sound = nullptr;

Sound *Sound::getInstance()
{
    if (!s_sound) {
        s_sound = new(std::nothrow) Sound();
        if (s_sound) {
            s_sound->init();
        }
    }
    return s_sound;
}

bool Sound::init()
{
    std::string d = SOUND_DIR;

    _pathMap["background"] = d + "Background.mp3";
    _pathMap["capture"] = d + "Capture.mp3";
    _pathMap["check"] = d + "Check.mp3";
    _pathMap["click"] = d + "Click.mp3";
    _pathMap["win"] = d + "Win.mp3";
    _pathMap["lose"] = d + "Lose.mp3";
    _pathMap["draw"] = d + "Draw.mp3";
    _pathMap["illegal"] = d + "Illegal.mp3";
    _pathMap["move"] = d + "Move.mp3";
    _pathMap["replay"] = d + "Replay.mp3";
    _pathMap["undo"] = d + "Undo.mp3";

    _audio = SimpleAudioEngine::getInstance();

    for (auto &path : _pathMap) {
        if (path.first == "background")
            _audio->preloadBackgroundMusic(path.second.c_str());
        else
            _audio->preloadEffect(path.second.c_str());
    }

    _audio->setBackgroundMusicVolume(0.5);
    _audio->setEffectsVolume(1);

    return true;
}

void Sound::playBackgroundMusic()
{
    _audio->playBackgroundMusic(_pathMap["background"].c_str(), true);
}
void Sound::stopBackgroundMusic()
{
    _audio->stopBackgroundMusic(true);
}
void Sound::playEffect(std::string effect)
{
    if (!_effectEnable)
        return;

    auto iter = _pathMap.find(effect);
    if (iter != _pathMap.end()) {
        _audio->playEffect(_pathMap[effect].c_str());
    }
}
void Sound::setEffectEnabled(bool enable)
{
    _effectEnable = enable;
}
