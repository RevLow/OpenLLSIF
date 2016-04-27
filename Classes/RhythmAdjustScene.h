//
//  RhythmAdjustScene.h
//  OpenLLSIF
//
//  Created by Tetsushi on 2016/04/27.
//
//

#ifndef __OpenLLSIF__RhythmAdjustScene__
#define __OpenLLSIF__RhythmAdjustScene__

#include <iostream>
USING_NS_CC;


class RhythmAdjustScene : public Layer
{
public:
    static Scene* createScene();
    virtual bool init();
    
    
    CREATE_FUNC(RhythmAdjustScene);
    virtual void onEnterTransitionDidFinish();
private:
    //タップの判定を行う正しい判定時間(ms)
    const int TAP_MILLI_SEC[8] = {6000, 8000, 10000, 12000, 14000, 16000, 18000, 20000};
    //音声ファイル名
    const std::string CHECK_AUDIO_FILE_NAME = "Sound/BGM/m_tune1.mp3";
    void CreateTapFx(Vec2 position);
    //タップイベント
    void onTouchesBegan(const std::vector<Touch *> &touches, cocos2d::Event *unused_event);
    void onTouchesEnded(const std::vector<Touch *> &touches, cocos2d::Event *unused_event);
};


#endif /* defined(__OpenLLSIF__RhythmAdjustScene__) */
