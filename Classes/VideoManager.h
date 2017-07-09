//
//  VideoManager.h
//  AvVideoSample
//
//  Created by RevLow on 2016/11/23.
//
//

#ifndef __AvVideoSample__VideoManager__
#define __AvVideoSample__VideoManager__

namespace VideoManager
{
    void play(const std::string& filePath, std::function<void(void)> func = nullptr);
    void pause();
    void resume();
    void stop();
    void seekTo(const float);
    void seekBy(const float);
}
#endif /* defined(__AvVideoSample__VideoManager__) */
