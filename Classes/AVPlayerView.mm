//
//  PlayerView.m
//  AvVideoSample
//
//  Created by RevLow on 2016/11/23.
//
//

#import "AVPlayerView.h"

@implementation AVPlayerView

- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        // Initialization code
    }
    return self;
}

+ (Class)layerClass
{
    return AVPlayerLayer.class;
}
/*
// Only override drawRect: if you perform custom drawing.
// An empty implementation adversely affects performance during animation.
- (void)drawRect:(CGRect)rect
{
    // Drawing code
}
*/

@end
