//
//  HomeScene.h
//  OpenLLSIF
//
//  Created by RevLow on 2015/07/13.
//
//

#ifndef __OpenLLSIF__HomeScene__
#define __OpenLLSIF__HomeScene__

#include "cocos2d.h"

class HomeScene : public cocos2d::Layer
{
public:
    // there's no 'id' in cpp, so we recommend returning the class instance pointer
    static cocos2d::Scene* createScene();
    
    // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
    virtual bool init();
    
    // implement the "static create()" method manually
    CREATE_FUNC(HomeScene);
    
    virtual void onEnterTransitionDidFinish();
};

#endif /* defined(__OpenLLSIF__HomeScene__) */
