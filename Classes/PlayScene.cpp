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
#include "UIVideoPlayer.h"
#include <thread>

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
    
    //ビデオを再生するためのレイヤーを追加
    //ただし、cocos2dのコードそのままだと最前面にビデオが来てしまうため
    //http://discuss.cocos2d-x.org/t/enhancement-request-for-videoplayer/16024
    //を参考にコードを変更する
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto vidPlayer = cocos2d::experimental::ui::VideoPlayer::create();
    vidPlayer->setContentSize(visibleSize);
    vidPlayer->setPosition(Vec2(480, 320));
    vidPlayer->setKeepAspectRatioEnabled(true);
    this->addChild(vidPlayer, -1);
    vidPlayer->setName("VideoLayer");
    vidPlayer->setFileName("/Users/tetsushi2/Documents/Develop/Video/Snow_Halation_PV.mp4");
    
    LayerColor *layer = LayerColor::create(Color4B::BLACK, 960, 640);
    layer->setOpacity(0);
    layer->setName("BlackLayer");
    this->addChild(layer);
    
    Sprite* backgroundSprite = Sprite::create("res/Image/hard_background.png");
    backgroundSprite->setName("backgroundImage");
    backgroundSprite->setPosition(480, 320);
    backgroundSprite->setLocalZOrder(-1);
    this->addChild(backgroundSprite);
    
    auto playScene = CSLoader::getInstance()->createNode("res/PlayScene.csb");
    playScene->setName("PlayLayer");
    playScene->setLocalZOrder(0);
    playScene->setOpacity(0);
    //playScene->setVisible(false);
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
                                                                                                    //playSplashはいらないので消去
                                                                                                    this->removeChild(playSplash);
                                                                                                    //ゲームを開始する
                                                                                                    this->Run();
                                                                                                }), NULL));
    
                                });
    playSplash->runAction(action);
    action->gotoFrameAndPlay(0, false);
    
    this->BPM = 86;
    
    
    return true;
}

/*
ゲームを開始する
 */
void PlayScene::Run()
{
    auto playScene = this->getChildByName("PlayLayer");
    //背景画像を設定

    
    auto backgroundLayer = this->getChildByName<LayerColor*>("BlackLayer");
    backgroundLayer->runAction(FadeTo::create(0.5f, 200));
    
    auto backSprite = this->getChildByName<LayerColor*>("backgroundImage");
    backSprite->runAction(FadeTo::create(0.5f, 0));
    
    //透明度を戻した後、実行を行う
    playScene->runAction(Sequence::create(
                                          FadeTo::create(0.5f, 255),
                                          CallFunc::create([this]()
                                                        {
                                                            auto videoLayer = this->getChildByName<experimental::ui::VideoPlayer*>("VideoLayer");
                                                            videoLayer->play();
                                                            //音楽の再生
                                                            //CocosDenshion::SimpleAudioEngine::getInstance()->playBackgroundMusic("Sound/BGM/Snow_halation.mp3");
                                                            
                                                            CocosDenshion::SimpleAudioEngine::getInstance()->preloadEffect("Sound/SE/Perfect.mp3");
                                                            //八分音符でのtickを計算
                                                            float tick = (float)60/((float)BPM);
                                                            CCLOG("TICK: %f", tick);
                                                            this->schedule(schedule_selector(PlayScene::PlayGame), tick);
                                                        })
                                          , NULL));
}
void PlayScene::PlayGame(float deltaT)
{
    CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("Sound/SE/Perfect.mp3");
}

/*
 バックグラウンドから復帰したときに呼ぶ関数
 AppDelegateで呼ぶように指定してある
 */
void PlayScene::applicationWillEnterForeground()
{
    auto videoLayer =  this->getChildByName<experimental::ui::VideoPlayer*>("VideoLayer");
    videoLayer->play();
}
