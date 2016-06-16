//
//  RhythmAdjustScene.cpp
//  OpenLLSIF
//
//  Created by Tetsushi on 2016/04/27.
//
//

#include "RhythmAdjustScene.h"
#include "ui/cocosGui.h"
#include "cocostudio/CocoStudio.h"
#include "HomeScene.h"
#include "StopWatch.h"
#include "AudioManager.h"
#include <algorithm>

template <typename T>
T median(std::vector<T>& c)
{
    size_t n = c.size() / 2;
    std::nth_element(c.begin(), c.begin() + n, c.end());
    return c[n];
}

Scene* RhythmAdjustScene::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();
    
    // 'layer' is an autorelease object
    auto layer = RhythmAdjustScene::create();
    
    // add layer as a child to scene
    scene->addChild(layer);
    
    // return the scene
    return scene;
}

bool RhythmAdjustScene::init()
{
    if(!Layer::init())
    {
        return false;
    }
    
    FileUtils::getInstance()->addSearchPath("res");
    
    Sprite* backgroundSprite = Sprite::create("background/background_easy.png");
    backgroundSprite->setPosition(Director::getInstance()->getVisibleSize().width/2, Director::getInstance()->getVisibleSize().height / 2);
    addChild(backgroundSprite);
    
    auto playScene = CSLoader::getInstance()->createNode("res/AdjustScene.csb");
    playScene->setName("PlayLayer");

    playScene->setLocalZOrder(0);
    addChild(playScene);
    
   
    
    auto action = cocostudio::timeline::ActionTimelineCache::getInstance()->createAction("res/AdjustScene.csb");
    Sprite* music_notes = playScene->getChildByName<Sprite*>("music_icon_7");
    music_notes->runAction(action);
    action->gotoFrameAndPlay(0, true);
    
    Sprite* five_sprite = playScene->getChildByName<Sprite*>("5");
    auto areaSize = five_sprite->getContentSize();
    
    tapArea = Circle::create(five_sprite->getPosition(), areaSize.width);
    tapArea->retain();
    AudioManager::getInstance()->preload(CHECK_AUDIO_FILE_NAME);
#ifdef DEBUG_DEBUG
    DrawNode *nodes = tapArea->getDrawNode(Color4F::Color4F(1.0f, 0.0f, 1.0f, 1.0f));
    nodes->setPosition(five_sprite->getPosition());
    this->addChild(nodes);
#endif
    
    //タッチイベントリスナーを作成
    auto listener = cocos2d::EventListenerTouchAllAtOnce::create();
    listener->setEnabled(true);
    listener->onTouchesBegan = CC_CALLBACK_2(RhythmAdjustScene::onTouchesBegan, this);
    //listener->onTouchesEnded = CC_CALLBACK_2(RhythmAdjustScene::onTouchesEnded, this);
    this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, this);
    
    return true;
}



void RhythmAdjustScene::onEnterTransitionDidFinish()
{
    
    AudioManager::getInstance()->play(CHECK_AUDIO_FILE_NAME
                                      , AudioManager::BGM);
    AudioManager::getInstance()->setOnExitCallback(CC_CALLBACK_2(RhythmAdjustScene::finishCallBack, this));
    StopWatch::getInstance()->start();
    
}

void RhythmAdjustScene::finishCallBack(int audioID, std::string fileName)
{
    float medianValue = median(delta_times);
    tapArea->release();
    CCLOG("%lf", medianValue);
    UserDefault::getInstance()->setFloatForKey("LATENCY", medianValue);
    Director::getInstance()->purgeCachedData();
    
    Scene* scene = HomeScene::createScene(ViewScene::Home);
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene, Color3B::BLACK));
    StopWatch::getInstance()->stop();
}


void RhythmAdjustScene::CreateTapFx(Vec2 position)
{
    auto tapFx = CSLoader::getInstance()->createNode("res/tapFx.csb");
    tapFx->setLocalZOrder(0);
    /*座標変換*/
    Vec2 rePosition = position;
    rePosition.x -= tapFx->getContentSize().width / 2;
    rePosition.y -= tapFx->getContentSize().height / 2;
    tapFx->setPosition(rePosition);
    this->addChild(tapFx);
    cocostudio::timeline::ActionTimeline* action = cocostudio::timeline::ActionTimelineCache::getInstance()->createAction("res/tapFx.csb");
    action->setLastFrameCallFunc([tapFx, this](){
        this->removeChild(tapFx);
    });
    tapFx->runAction(action);
    action->gotoFrameAndPlay(0, false);
    
}

void RhythmAdjustScene::onTouchesBegan(const std::vector<Touch *> &touches, cocos2d::Event *unused_event)
{
    CCLOG("CALL");
    if(tapArea->containsPoint(touches[0]->getLocation()))
    {
        double now = StopWatch::getInstance()->currentTime();
        double delta = 0;
        if(index < 8)
        {
            double correctTime = TAP_MILLI_SEC[index];
            
            delta = correctTime - now;
            delta_times.push_back(delta);
            Vec2 v = tapArea->getPosition();
            CreateTapFx(v);
            index++;
        }
        
        double val = median(delta_times);
        CCLOG("NOW: %lf, MEDIAN: %lf", delta, val);
    }
}

float RhythmAdjustScene::medianCalculation()
{
    float size = delta_times.size();
    std::vector<float> _v(delta_times.size());
    copy(delta_times.begin(), delta_times.end(), back_inserter(_v));
    float tmp;
    for (int i = 0; i < size - 1; i++){
        for (int j = i + 1; j < size; j++) {
            if (_v[i] > _v[j]){
                tmp = _v[i];
                _v[i] = _v[j];
                _v[j] = tmp;
            }
        }
    }
    float medianValue;
    if (_v.size() % 2 == 1) {
        medianValue = _v[(_v.size() - 1) / 2];
    } else {
        medianValue = (_v[(_v.size() / 2) - 1] + _v[_v.size() / 2]) / 2;
    }
    
    //最終的な誤差を代入
    return medianValue;
}
