//
//  AudioManager.h
//  OpenLLSIF
//
//  Created by RevLow on 2016/01/17.
//
//

#ifndef __OpenLLSIF__AudioManager__
#define __OpenLLSIF__AudioManager__
#include "cocos2d.h"

class AudioManager
{
public:
    enum AudioType
    {
        BGM,
        SE,
    };
    
    static AudioManager* getInstance();
    void init();
    
    void play(std::string &filePath,AudioType type, bool loop = false);
    void stop(AudioType type);
    void pause(AudioType type);
    void resume(AudioType type);
    
    CC_PROPERTY(float, _bgmVolume, BgmVolume);
    CC_PROPERTY(float, _seVolume, SeVolume);
    void setVolume(float volume);
    
    CC_PROPERTY(bool, _mute, Mute);
    
    void saveParams();
    void setOnExitCallback(const std::function<void(int, const std::string&)> &callbackFunc);
    bool isPlaying();
    float getCurrentTime();
    void preload(const std::string& filePath);
    //time_t getStartTime();
private:
    struct BgmData
    {
        BgmData()
        {
            bgmId = -1;
            type = BGM;
            playing = false;
        }
        int bgmId;
        AudioType type;
        bool playing;
    };
    
    AudioManager();
    BgmData _bgmData;
    
    std::vector<int> _seList;
    
    float getMuteBgmVolume(){return _mute ? 0.0f : _bgmVolume;};
    float getMuteSeVolume(){return _mute ? 0.0f : _seVolume;};
};
#endif /* defined(__OpenLLSIF__AudioManager__) */
