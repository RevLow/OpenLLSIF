//
//  HomeScene.cpp
//  OpenLLSIF
//
//  Created by RevLow on 2015/07/13.
//
//

#include "HomeScene.h"
#include "ui/cocosGui.h"
#include "cocostudio/CocoStudio.h"
#include "SimpleAudioEngine.h"

USING_NS_CC;

Scene* HomeScene::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();
    
    // 'layer' is an autorelease object
    auto layer = HomeScene::create();
    
    // add layer as a child to scene
    scene->addChild(layer);
    
    // return the scene
    return scene;
}

bool HomeScene::init()
{
    if (!Layer::init()) {
        return false;
    }
    
    //背景レイヤーの作成
    //ここの階層を変えていく
    auto background = CSLoader::getInstance()->createNode("res/home_background.csb");
    background->setName("backgroundLayer");
    auto action = cocostudio::timeline::ActionTimelineCache::getInstance()->createAction("res/home_background.csb");
    
    //背景レイヤーのアニメーションを実行
    background->runAction(action);
    action->gotoFrameAndPlay(0, true);
    //明示的にz座標を指定
    background->setLocalZOrder(0);
    addChild(background);
    
    //ボタンのレイヤーを作成
    auto homeScene = CSLoader::getInstance()->createNode("res/HomeScene.csb");
    homeScene->setName("HomeSceneLayer");
    homeScene->setLocalZOrder(1);
    addChild(homeScene);
    
    
    //各種ボタンの設定
    //ホームボタンの設定
    auto homeButton = homeScene->getChildByName<ui::Button*>("Home_Button");
    homeButton->addClickEventListener([this](Ref *ref)
    {
        auto background = this->getChildByName("backgroundLayer");
        
        
        background->runAction(Sequence::create(FadeTo::create(0.3f, 0),CallFunc::create([&]()
        {
            this->removeChildByName("backgroundLayer");
            auto newLayer = CSLoader::getInstance()->createNode("res/home_background.csb");
            auto action = cocostudio::timeline::ActionTimelineCache::getInstance()->createAction("res/home_background.csb");
            newLayer->setName("backgroundLayer");
            
            newLayer->runAction(action);
            action->gotoFrameAndPlay(0, true);
            this->addChild(newLayer, 0);
        }), NULL));
        
        CocosDenshion::SimpleAudioEngine::getInstance()->preloadEffect("Sound/SE/decide.mp3");
        CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("Sound/SE/decide.mp3");
        
    });
    
    //ライブボタンの設定
    auto liveButton = homeScene->getChildByName<ui::Button*>("Live_Button");
    liveButton->addClickEventListener([this](Ref *ref)
                                      {
                                          auto background = this->getChildByName("backgroundLayer");
                                          
                                          background->runAction(Sequence::create(FadeTo::create(0.3f, 0),CallFunc::create([this]()
                                                                                                                          {
                                                                                                                              this->removeChildByName("backgroundLayer");
                                                                                                                              auto newLayer = CSLoader::getInstance()->createNode("res/song_selection.csb");
                                                                                                                              auto action = cocostudio::timeline::ActionTimelineCache::getInstance()->createAction("res/song_selection.csb");
                                                                                                                              newLayer->setName("backgroundLayer");
                                                                                                                              
                                                                                                                              newLayer->runAction(action);
                                                                                                                              action->gotoFrameAndPlay(0,true);
                                                                                                                              this->addChild(newLayer, 0);
                                                                                                                          }), NULL));
                                          
                                          CocosDenshion::SimpleAudioEngine::getInstance()->preloadEffect("Sound/SE/decide.mp3");
                                          CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("Sound/SE/decide.mp3");
                                          
                                      });
    
    
    return true;
}

void HomeScene::onEnterTransitionDidFinish()
{
    
    auto homeScene = this->getChildByName("HomeSceneLayer");
    auto action = cocostudio::timeline::ActionTimelineCache::getInstance()->createAction("res/HomeScene.csb");
    
    homeScene->runAction(action);
    action->gotoFrameAndPlay(0, false);
    CocosDenshion::SimpleAudioEngine::getInstance()->preloadBackgroundMusic("Sound/BGM/title_bgm.mp3");
    CocosDenshion::SimpleAudioEngine::getInstance()->playBackgroundMusic("Sound/BGM/title_bgm.mp3", true);

}