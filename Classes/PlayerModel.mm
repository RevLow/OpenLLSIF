//
//  PlayerModel.cpp
//  AvVideoSample
//
//  Created by RevLow on 2016/11/23.
//
//

#include "PlayerModel.h"

@implementation PlayerModel

static PlayerModel* _sharedModel = nil;

+ (PlayerModel*) sharedModel
{
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        _sharedModel = [PlayerModel new];
    });
    return _sharedModel;
}

- (id) init
{
    self = [super init];

    if (self)
    {
        
    }
    
    return self;
}

@end