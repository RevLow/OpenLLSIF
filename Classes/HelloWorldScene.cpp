#include "HelloWorldScene.h"
#include "ui/cocosGui.h"
#include "cocostudio/CocoStudio.h"
#include <SimpleAudioEngine.h>

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
    
    //////////////////////////////
    //2, シーンの読み込み
    //
    
    FileUtils::getInstance()->addSearchPath("res");
    auto topScene = CSLoader::getInstance()->createNode("res/MainScene.csb");
    
    auto view = Director::getInstance()->getOpenGLView();
    auto frame = view->getFrameSize();
    
    //Retinaじゃない場合
    if(frame.width <= 480)
    {
        //topScene->setAnchorPoint(Vec2(0.0,1.0));
        topScene->setPosition(0, 0);
        topScene->setScale(0.5);
    }

    addChild(topScene);
    
    
    //////////////////////////////
    //3, BGM再生
    //
    CocosDenshion::SimpleAudioEngine::getInstance()->playBackgroundMusic("Sound/BGM/paradice_live.mp3");
    
    //////////////////////////////
    //4, アニメーション読み込み
    //
    auto action = cocostudio::timeline::ActionTimelineCache::getInstance()->createAction("res/MainScene.csb");
    auto textLabel = topScene->getChildByName<Label*>("Text_1");
    
    textLabel->runAction(action);
    action->gotoFrameAndPlay(0, true);
    
    return true;
}