//
//  RhythmAdjustScene.h
//  OpenLLSIF
//
//  Created by Tetsushi on 2016/04/27.
//
//

#ifndef __OpenLLSIF__RhythmAdjustScene__
#define __OpenLLSIF__RhythmAdjustScene__

#include <vector>
#include "Note.h"

USING_NS_CC;


class RhythmAdjustScene : public Layer
{
public:
    static Scene* createScene();
    virtual bool init();
    
    
    CREATE_FUNC(RhythmAdjustScene);
    virtual void onEnterTransitionDidFinish();
    void finishCallBack();
private:
    int index = 0;
    //タップの判定を行う正しい判定時間(ms)
    const int TAP_MILLI_SEC[8] = {6036, 8036, 10036, 12036, 14036, 16036, 18036, 20036};
    std::vector<double> delta_times;
    Circle *tapArea;
    //音声ファイル名
    const std::string CHECK_AUDIO_FILE_NAME = "Sound/BGM/m_tune1.mp3";
    void CreateTapFx(Vec2 position);
    float medianCalculation();
    //Vec2 _fxPoints;
    
    //タップイベント
    void onTouchesBegan(const std::vector<Touch *> &touches, cocos2d::Event *unused_event);
    //void onTouchesEnded(const std::vector<Touch *> &touches, cocos2d::Event *unused_event);
    //void update(float delta);
};


#endif /* defined(__OpenLLSIF__RhythmAdjustScene__) */
