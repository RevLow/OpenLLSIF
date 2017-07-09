//
//  VideoManager.mm
//  AvVideoSample
//
//  Created by RevLow on 2016/11/23.
//
//

#include "VideoManager.h"
#include "cocos2d.h"

#import "AVPlayerView.h"
#import "PlayerModel.h"

USING_NS_CC;

@interface VideoController : NSObject
{
}
@end

@implementation VideoController
{
    PlayerModel* model;
    std::function<void(void)> callbackFunc;
}

- (id)init
{
    self = [super init];
    if (self)
    {
        model = [PlayerModel sharedModel];
    }

    return self;
}

- (void) play :(NSString*) filePath onLoaded: (std::function<void(void)>) func
{
    NSURL* url = [NSURL fileURLWithPath:filePath];
    AVURLAsset* asset = [[AVURLAsset alloc] initWithURL:url options:nil];
    [asset loadValuesAsynchronouslyForKeys:@[@"playable"] completionHandler:^{
        
    }];
    AVPlayerItem* playerItem = [[AVPlayerItem alloc] initWithURL:url];
    model.player = [[AVPlayer alloc] initWithPlayerItem:playerItem];
    model.playerView = [[AVPlayerView alloc] initWithFrame:model.mainUiView.bounds];
    [(AVPlayerLayer*)model.playerView.layer setPlayer: model.player];
    [model.mainUiView insertSubview:model.playerView atIndex:0];

    callbackFunc = func;
    [model.player addObserver:self forKeyPath:@"status" options:NSKeyValueObservingOptionNew context:nil];
    [NSNotificationCenter.defaultCenter addObserver:self
                                           selector:@selector(handleEndPlay)
                                               name:AVPlayerItemDidPlayToEndTimeNotification
                                             object:playerItem];
    
}

- (void) pause
{
    if(model.player.rate == 0.0)
    {
        return;
    }
    
    [model.player pause];
}

- (void) stop
{
    [model.player pause];
    [self killMoviePlayer];
}

- (void) resume
{
    if (model.player.rate != 0.0)
    {
        return;
    }

    [model.player play];
}

- (void) seekTo: (float) time
{
    CMTime duration = [[[model.player currentItem] asset] duration];
    Float64 maxSeconds = CMTimeGetSeconds(duration);

    if (time > maxSeconds * 1000.0f)
    {
        return;
    }
    
    CMTime millisecTime = CMTimeMake(time, 1000);
    [model.player seekToTime:millisecTime toleranceBefore:kCMTimeZero toleranceAfter:kCMTimeZero];
}

- (void) seekBy: (float) delta
{
    CMTime now = [model.player currentTime];
    Float64 nowSec = now.value / now.timescale;
    nowSec *= 1000.0f;
    nowSec += delta;

    CMTime millisec = CMTimeMake(nowSec, 1000);
    [model.player seekToTime:millisec toleranceBefore:kCMTimeZero toleranceAfter:kCMTimeZero];
}

- (void) killMoviePlayer
{
    [((AVPlayerLayer*)model.playerView.layer) removeFromSuperlayer];
    model.player = nil;
    [model.playerView removeFromSuperview];
    model.playerView = nil;
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if (model.player.status == AVPlayerItemStatusReadyToPlay) {
        [model.player removeObserver:self forKeyPath:@"status"];
        [model.player play];

        if (callbackFunc != nullptr)
        {
            callbackFunc();
            callbackFunc = nullptr;
        }
    
        return;
    }
    
    [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
}

- (void)handleEndPlay
{
    [self stop];
}

@end

namespace{
    VideoController* controller = [[VideoController alloc] init ];
}

/**
 *  ファイルをロードし再生を行う
 *
 *  @param filePath               再生するファイルの場所
 *  @param func                   再生を開始したときに実行するコールバック関数
 */
void VideoManager::play(const std::string& filePath, std::function<void(void)> func)
{
    NSString* path = [[NSString alloc] initWithCString:filePath.c_str() encoding:NSUTF8StringEncoding];
    [controller play:path onLoaded:func];
}

/**
 *  再生中のビデオを一時停止する
 */
void VideoManager::pause()
{
    [controller pause];
}

/**
 *  一時停止から復帰して、再生する
 */
void VideoManager::resume()
{
    [controller resume];
}

/**
 *  再生を終了する
 */
void VideoManager::stop()
{
    [controller stop];
}

/**
 *  指定時間までシークする
 *
 *  @param time シーク時間(ミリ秒)
 */
void VideoManager::seekTo(const float time)
{
    [controller seekTo:time];
}

/**
 *  現在時間を基準に指定時間分シークする
 *
 *  @param delta 増加量(ミリ秒)
 */
void VideoManager::seekBy(const float delta)
{
    [controller seekBy:delta];
}
