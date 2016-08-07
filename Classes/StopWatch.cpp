//
//  StopWatch.cpp
//  OpenLLSIF
//
//  Created by RevLow on 2016/01/20.
//
//

#include "StopWatch.h"
#include "AudioManager.h"
#include <mutex>
static std::mutex s_mutex;
static StopWatch *_instance;

StopWatch* StopWatch::getInstance()
{
    if (!_instance)
    {
        bool rtn = _instance->init();
        if (!rtn)
        {
            return nullptr;
        }
    }
    return _instance;
}

bool StopWatch::init()
{
    std::lock_guard<std::mutex> lock(s_mutex);
    _instance = new StopWatch();
    
    if (_instance == nullptr)
    {
        return false;
    }
    
    return true;
}

StopWatch::StopWatch() : _start(std::chrono::system_clock::now()), _totalSleepTime(0.0), _status(STOPPED)
{
}

StopWatch::~StopWatch()
{
    if (_instance!=nullptr)
    {
        delete _instance;
    }
}

void StopWatch::start()
{
    _start = std::chrono::system_clock::now();
    _status = PLAYING;
}
void StopWatch::stop()
{
    _start = std::chrono::system_clock::now();
    _totalSleepTime= 0.0;
    _status = STOPPED;
}

void StopWatch::pause()
{
    _pauseTimeStart = std::chrono::system_clock::now();
    _status = PAUSED;
}
void StopWatch::resume()
{
    std::chrono::system_clock::time_point p = std::chrono::system_clock::now();
    _totalSleepTime += std::chrono::duration_cast<std::chrono::milliseconds>(p - _pauseTimeStart).count();
    _status = PLAYING;
}

double StopWatch::currentTime()
{
    std::lock_guard<std::mutex> lock(s_mutex);
    if (_status == PAUSED)
    {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(_pauseTimeStart - _start).count();
        return elapsed;
    }
    
    auto now = std::chrono::system_clock::now();
    double elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - _start).count() - _totalSleepTime;

    return elapsed;
}