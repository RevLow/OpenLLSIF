#include "HelloWorldScene.h"
#include "HomeScene.h"
#include "ui/cocosGui.h"
#include "cocostudio/CocoStudio.h"
#include <LLAudioEngine/LLAudioEngine.h>
//#include "SimpleAudioEngine.h"
//#include "AudioManager.h"

USING_NS_CC;

Scene* HelloWorld::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();
    
    // 'layer' is an autorelease object
    auto layer = HelloWorld::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Layer::init() )
    {
        return false;
    }
    
    
    //背景色の設定
    Size winSize(960, 640);
    auto bg = LayerColor::create(Color4B::WHITE, winSize.width, winSize.height);
    this->addChild(bg);
    
    //////////////////////////////
    //2, シーンの読み込み
    //
    
    FileUtils::getInstance()->addSearchPath("res");
    auto topScene = CSLoader::getInstance()->createNode("res/MainScene.csb");


    this->setColor(Color3B(255, 255, 255));
    topScene->setOpacity(0);
    addChild(topScene);
    
    
    //////////////////////////////
    //3, BGM再生
    //

    std::string filePath = "Sound/BGM/paradice_live.mp3";
    std::string fullPath = FileUtils::getInstance()->fullPathForFilename(filePath);
//    CocosDenshion::SimpleAudioEngine::getInstance()->setBackgroundMusicVolume(0.8);
//    CocosDenshion::SimpleAudioEngine::getInstance()->setEffectsVolume(1.0);
//    CocosDenshion::SimpleAudioEngine::getInstance()->playBackgroundMusic(fullPath.c_str(), true);
    LLAudioEngine::getInstance()->setBackgroundMusic(1.0);
    LLAudioEngine::getInstance()->setEffectVolume(0.9);
    LLAudioEngine::getInstance()->playBackgroundMusic(fullPath.c_str(), true);
    //AudioManager::getInstance()->setSeVolume(1.5);
    //AudioManager::getInstance()->play(fullPath, AudioManager::BGM, true);
    //////////////////////////////
    //4, アニメーション読み込み
    //
    auto action = cocostudio::timeline::ActionTimelineCache::getInstance()->createAction("res/MainScene.csb");
    auto textLabel = topScene->getChildByName<Label*>("Text_1");
    
    textLabel->runAction(action);
    action->gotoFrameAndPlay(0, true);
    
    topScene->runAction(FadeTo::create(0.5, 255));
    
    
    //////////////////////////////
    //5, ボタンの設定
    //
    
    auto nextButton = topScene->getChildByName<ui::Button*>("nextButton");
    nextButton->setVisible(true);
    nextButton->setOpacity(0);
    
    //ボタンイベントの作成
    nextButton->addClickEventListener([](Ref* ref)
    {
        //CocosDenshion::SimpleAudioEngine::getInstance()->preloadEffect("Sound/SE/decide.mp3");
        //CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("Sound/SE/decide.mp3");
        
        //CocosDenshion::SimpleAudioEngine::getInstance()->stopBackgroundMusic();
        std::string filePath = "Sound/SE/decide.mp3";
        std::string fullPath = FileUtils::getInstance()->fullPathForFilename(filePath);
        //AudioManager::getInstance()->play(fullPath, AudioManager::AudioType::SE);
        //AudioManager::getInstance()->stop(AudioManager::BGM);
//        CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(fullPath.c_str());
//        CocosDenshion::SimpleAudioEngine::getInstance()->stopBackgroundMusic();
        LLAudioEngine::getInstance()->playEffect(fullPath.c_str());
        LLAudioEngine::getInstance()->stopBackgroundMusic();
        
        //次のシーンを読み込む
        auto scene = HomeScene::createScene(ViewScene::Home);
        //シーンの移動
        Director::getInstance()->replaceScene(TransitionFade::create(0.5, scene));
    });
    
    return true;
}