//
//  PlayerModel.h
//  AvVideoSample
//
//  Created by RevLow on 2016/11/23.
//
//

#ifndef __AvVideoSample__PlayerModel__
#define __AvVideoSample__PlayerModel__

#import <AVFoundation/AVPlayer.h>
#import <AVFoundation/AVPlayerItem.h>
#import <AVFoundation/AVPlayerLayer.h>
#import "AVPlayerView.h"

@interface PlayerModel : NSObject
{
}

@property (strong, nonatomic) UIView* mainUiView;
@property (strong, nonatomic) AVPlayer* player;
@property (strong, nonatomic) AVPlayerView* playerView;

+ (PlayerModel*) sharedModel;

@end
#endif /* defined(__AvVideoSample__PlayerModel__) */
