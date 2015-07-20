//
//  PlayScene.cpp
//  OpenLLSIF
//
//  Created by RevLow on 2015/07/20.
//
//

#include "PlayScene.h"
#include "ui/cocosGui.h"
#include "cocostudio/CocoStudio.h"
#include "SimpleAudioEngine.h"


Scene* PlayScene::createScene(std::string playSongFile)
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();
    
    // 'layer' is an autorelease object
    auto layer = PlayScene::create(playSongFile);
    
    // add layer as a child to scene
    scene->addChild(layer);
    
    // return the scene
    return scene;
}

bool PlayScene::init(std::string playSongFile)
{
    if(!Layer::init())
    {
        return false;
    }
    
    //背景画像を設定
    Sprite* backgroundSprite = Sprite::create("res/Image/hard_background.png");
    backgroundSprite->setName("backgroundImage");
    backgroundSprite->setPosition(480, 320);
    backgroundSprite->setLocalZOrder(-1);
    this->addChild(backgroundSprite);
    
    auto playScene = CSLoader::getInstance()->createNode("res/PlayScene.csb");
    playScene->setLocalZOrder(0);
    playScene->setVisible(false);
    this->addChild(playScene);
    
    //ゲーム開始時のアニメーション用のレイヤーを重ねる
    auto playSplash = CSLoader::getInstance()->createNode("res/splash_layer.csb");
    playScene->setLocalZOrder(1);
    this->addChild(playSplash);
    
    //アニメーションを動かす
    auto action = cocostudio::timeline::ActionTimelineCache::getInstance()->createAction("res/splash_layer.csb");
    
    //背景レイヤーのアニメーションを実行
    action->setLastFrameCallFunc([playSplash,this]()
                                {
                                    playSplash->runAction(Sequence::create(FadeTo::create(1.0f, 0),
                                                                           CallFunc::create([this, playSplash]()
                                                                                                {
                                                                                                    //この辺でゲームを開始するスケジュールを実行すればOK
                                                                                                    //playSplashはいらないので消去
                                                                                                    this->removeChild(playSplash);
                                                                                                }), NULL));
    
                                });
    playSplash->runAction(action);
    action->gotoFrameAndPlay(0, false);
    
    
    return true;
}