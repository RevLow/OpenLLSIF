//
//  AudioManager.cpp
//  OpenLLSIF
//
//  Created by RevLow on 2016/01/17.
//
//
#include "AudioManager.h"

USING_NS_CC;
using namespace experimental;

static AudioManager* _instance;

AudioManager* AudioManager::getInstance()
{
    if (!_instance) {
        _instance = new AudioManager();
        _instance->init();
    }
    return _instance;
}

AudioManager::AudioManager()
:_bgmData(BgmData())
, _seList(std::vector<int>())
, _bgmVolume(1.0)
, _seVolume(1.0)
{
    
}

void AudioManager::init()
{
    _mute = UserDefault::getInstance()->getBoolForKey("mute", false);
    _bgmVolume = UserDefault::getInstance()->getFloatForKey("bgm_volume", 1.0f);
    _seVolume = UserDefault::getInstance()->getFloatForKey("se_volume", 1.0f);
    
}

void AudioManager::play(const std::string &filePath,AudioManager::AudioType type, bool loop)
{
    if (AudioManager::BGM == type)
    {
        if (_bgmData.playing)
        {
            AudioEngine::stop(_bgmData.bgmId);
        }
        int id = AudioEngine::play2d(filePath, loop, getMuteBgmVolume());
        _bgmData.bgmId = id;
        _bgmData.type = type;
        _bgmData.playing = true;
    }
    else
    {
        AudioProfile *profile = AudioEngine::getDefaultProfile();
        //profile->maxInstances = 2;
        //AudioEngine::setMaxAudioInstance(4);
        int id = AudioEngine::play2d(filePath, loop, getMuteSeVolume(), profile);
        _seList.push_back(id);
    }
    
}

void AudioManager::preload(const std::string& filePath)
{
    AudioEngine::preload(filePath);
}

void AudioManager::stop(AudioManager::AudioType type)
{
    if(_bgmData.playing && type == AudioManager::BGM)
    {
        AudioEngine::stop(_bgmData.bgmId);

        _bgmData.playing = false;
        _bgmData.type = BGM;
        _bgmData.bgmId = -1;
    }
}

void AudioManager::pause(AudioType type)
{
    if (_bgmData.playing && type == AudioManager::BGM)
    {
        AudioEngine::pause(_bgmData.bgmId);
    }
}
void AudioManager::resume(AudioType type)
{
    if (_bgmData.playing && type == AudioManager::BGM)
    {
        AudioEngine::resume(_bgmData.bgmId);
    }
}

void AudioManager::setBgmVolume(float var)
{
    _bgmVolume = var;
    if (_bgmData.playing) {
        AudioEngine::setVolume(_bgmData.bgmId, getMuteBgmVolume());
    }
}

float AudioManager::getBgmVolume()
{
    return _bgmVolume;
}

void AudioManager::setSeVolume(float var)
{
    _seVolume = var;
    for (auto id : _seList) {
        AudioEngine::setVolume(id, getMuteSeVolume());
    }
}

float AudioManager::getSeVolume()
{
    return _seVolume;
}

void AudioManager::setVolume(float volume)
{
    setBgmVolume(volume);
    setSeVolume(volume);
}

void AudioManager::setMute(bool mute)
{
    if (_mute != mute) {
        _mute = mute;
        
        setBgmVolume(getBgmVolume());
        setSeVolume(getSeVolume());
    }
    
}

bool AudioManager::getMute()
{
    return _mute;
}

void AudioManager::setOnExitCallback(const std::function<void(int,const std::string&)> &callbackFunc)
{
    AudioEngine::setFinishCallback(_bgmData.bgmId, callbackFunc);
}

void AudioManager::saveParams()
{
    UserDefault::getInstance()->setBoolForKey("mute", _mute);
    UserDefault::getInstance()->setFloatForKey("bgm_volume", _bgmVolume);
    UserDefault::getInstance()->setFloatForKey("se_volume", _seVolume);
    
}

bool AudioManager::isPlaying()
{
    AudioEngine::AudioState state = AudioEngine::getState(_bgmData.bgmId);
    if(state == AudioEngine::AudioState::PLAYING)
    {
        return true;
    }
    
    return false;
}

float AudioManager::getCurrentTime()
{
    return AudioEngine::getCurrentTime(_bgmData.bgmId);
}

/*
time_t AudioManager::getStartTime()
{
    return AudioEngine::getStartTime(_bgmData.bgmId);
}*/
