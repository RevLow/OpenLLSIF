//
//  StopWatch.h
//  OpenLLSIF
//
//  Created by RevLow on 2016/01/20.
//
//

#ifndef __OpenLLSIF__StopWatch__
#define __OpenLLSIF__StopWatch__

#include <iostream>
#include <chrono>

enum StopWatchStatus
{
    PLAYING,
    STOPPED,
    PAUSED
};

class StopWatch
{
public:
    static StopWatch* getInstance();
    bool init();
    void start();//最初の時間を設定
    void stop();//ストップウォッチを止める
    
    void pause();//一時停止
    void resume();//再開
    double currentTime();
    CC_SYNTHESIZE_READONLY(StopWatchStatus, _status, Status);
private:
    StopWatch();
    ~StopWatch();
    StopWatch(const StopWatch &other){}
    StopWatch &operator=(const StopWatch &other){}
    double _totalSleepTime;
    std::chrono::system_clock::time_point _start;
    std::chrono::system_clock::time_point _pauseTimeStart;
};



#endif /* defined(__OpenLLSIF__StopWatch__) */
