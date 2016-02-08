//
//  StopWatch.cpp
//  OpenLLSIF
//
//  Created by RevLow on 2016/01/20.
//
//

#include "StopWatch.h"
#include <mutex>
static std::mutex s_mutex;
static StopWatch *_instance;
bool isPause;
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

StopWatch::StopWatch() : _start(std::chrono::system_clock::now()), _totalSleepTime(0.0)
{
    isPause = false;
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
}
void StopWatch::stop()
{
    _start = std::chrono::system_clock::now();
    _totalSleepTime= 0.0;
}

void StopWatch::pause()
{
    _pauseTimeStart = std::chrono::system_clock::now();
    isPause = true;
}
void StopWatch::resume()
{
    std::chrono::system_clock::time_point p = std::chrono::system_clock::now();
    _totalSleepTime += std::chrono::duration_cast<std::chrono::milliseconds>(p - _pauseTimeStart).count();
    isPause = false;
}

double StopWatch::currentTime()
{
    std::lock_guard<std::mutex> lock(s_mutex);
    if (!isPause)
    {
        auto now = std::chrono::system_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - _start).count() - _totalSleepTime;
        return elapsed;
    }
    else
    {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(_pauseTimeStart - _start).count();
        return elapsed;
    }
}