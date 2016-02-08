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
    static cocos2d::Scene* createScene(ViewScene entryScene);
    
    // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
    virtual bool init(ViewScene entryScene);
    
    // implement the "static create()" method manually
    
    virtual void onEnterTransitionDidFinish();
protected:
    
    void homeButton_action(Ref *ref);
    void liveButton_action(Ref *ref);
    void loadLiveScene();
    void loadHomeScene();
    
    void nextAlbum_click(Ref *ref);
    void previousAlbum_click(Ref *ref);
    bool jacket_touch(cocos2d::Touch* touch, cocos2d::Event* e);
private:
    

    ViewScene _currentScene;
    
    /*
     ジャケットのタグ番号とファイルのパスの対応付けのためのMap
     */
    std::map<int, std::string> JacketInfoMap;
    //ジャケット内の先頭のインデックス番号を保持する
    int touchable_index;
};

#endif /* defined(__OpenLLSIF__HomeScene__) */
