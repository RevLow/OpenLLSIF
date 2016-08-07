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

USING_NS_CC;

enum ViewScene
{
    Home=0,
    Live
};

class HomeScene : public cocos2d::Layer, create_func<HomeScene>
{
public:
    using create_func::create;
    
    // there's no 'id' in cpp, so we recommend returning the class instance pointer
    static Scene* createScene(ViewScene entryScene);
    
    // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
    virtual bool init(ViewScene entryScene);

    virtual void onEnterTransitionDidFinish();
protected:
    void homeButtonAction(Ref *ref);
    void liveButtonAction(Ref *ref);
    void configButtonAction(Ref *ref);
    void nextAlbumClick(Ref *ref);
    void previousAlbumClick(Ref *ref);
    bool jacketTouchEvent(Touch* touch, Event* e);
private:
    void loadLiveScene();
    void loadHomeScene();
private:
    std::vector<std::string> _songStack;
    ViewScene _currentScene;
};

#endif /* defined(__OpenLLSIF__HomeScene__) */
