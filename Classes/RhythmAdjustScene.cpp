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
#include "StopWatch.h"
#include "AudioManager.h"

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
    
    Sprite* backgroundSprite = Sprite::create("background/background_easy.png");
    addChild(backgroundSprite);
    
    auto playScene = CSLoader::getInstance()->createNode("res/AdjustScene.csb");
    playScene->setName("PlayLayer");
    playScene->setLocalZOrder(0);
    playScene->setOpacity(0);
    this->addChild(playScene);
    
    auto action = cocostudio::timeline::ActionTimelineCache::getInstance()->createAction("res/AdjustScene.csb");
    Sprite* music_notes = playScene->getChildByName<Sprite*>("music_icon_7");
    music_notes->runAction(action);
    action->gotoFrameAndPlay(0, true);

    AudioManager::getInstance()->preload(CHECK_AUDIO_FILE_NAME);
    
    
    //タッチイベントリスナーを作成
    auto listener = cocos2d::EventListenerTouchAllAtOnce::create();
    listener->setEnabled(true);
    listener->onTouchesBegan = CC_CALLBACK_2(RhythmAdjustScene::onTouchesBegan, this);
    listener->onTouchesEnded = CC_CALLBACK_2(RhythmAdjustScene::onTouchesEnded, this);
    this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, this);
    
    return true;
}

void RhythmAdjustScene::onEnterTransitionDidFinish()
{
    
    
}

void RhythmAdjustScene::CreateTapFx(Vec2 position)
{
    std::vector<Sprite*> fx_array, inner_fx;
    
    fx_array.push_back(Sprite::createWithSpriteFrameName("Image/PlayUI/TapFx1.png"));
    fx_array.push_back(Sprite::createWithSpriteFrameName("Image/PlayUI/TapFx2.png"));
    fx_array.push_back(Sprite::createWithSpriteFrameName("Image/PlayUI/TapFx3.png"));
    
    inner_fx.push_back(Sprite::createWithSpriteFrameName("Image/PlayUI/TapFx1.png"));
    inner_fx.push_back(Sprite::createWithSpriteFrameName("Image/PlayUI/TapFx2.png"));
    inner_fx.push_back(Sprite::createWithSpriteFrameName("Image/PlayUI/TapFx3.png"));
    
    for (auto it = fx_array.begin(); it != fx_array.end();it++)
    {
        (*it)->setOpacity(0);
        (*it)->setBlendFunc((BlendFunc){GL_SRC_ALPHA, GL_ONE});
        //(*it)->setScale(0.5f);
        (*it)->setPosition(position);
        
        addChild(*it);
    }
    //削除
    auto remove = RemoveSelf::create(true);
    auto action = Spawn::create(FadeOut::create(0.1), ScaleTo::create(0.1, 2.5f),NULL);
    
    fx_array[0]->runAction(Sequence::create(FadeIn::create(0.05),
                                            EaseOut::create(action, 2.0),remove, NULL));
    
    action = Spawn::create(ScaleTo::create(0.1, 2.8f), FadeOut::create(0.184), NULL);
    fx_array[1]->runAction(Sequence::create(FadeIn::create(0),
                                            EaseOut::create(action, 2.0),
                                            remove, NULL));
    action = Spawn::create(ScaleTo::create(0.1, 3.0f), FadeOut::create(0.184), NULL);
    fx_array[2]->runAction(Sequence::create(FadeIn::create(0.0184),
                                            EaseOut::create(action, 2.0),
                                            remove, NULL));
    
    //iner_fxに対する処理
    for (auto it = inner_fx.begin(); it != inner_fx.end();it++)
    {
        (*it)->setOpacity(0);
        //(*it)->setBlendFunc(BlendFunc::ADDITIVE);
        //(*it)->setScale(0.5f);
        (*it)->setPosition(position);
        
        addChild(*it);
    }
    //削除
    action = Spawn::create(FadeOut::create(0.1), ScaleTo::create(0.1, 2.5f),NULL);
    
    inner_fx[0]->runAction(Sequence::create(FadeIn::create(0.05),
                                            EaseOut::create(action, 2.0),remove, NULL));
    
    action = Spawn::create(ScaleTo::create(0.1, 2.8f), FadeOut::create(0.184), NULL);
    inner_fx[1]->runAction(Sequence::create(FadeIn::create(0),
                                            EaseOut::create(action, 2.0),
                                            remove, NULL));
    action = Spawn::create(ScaleTo::create(0.1, 3.0f), FadeOut::create(0.184), NULL);
    inner_fx[2]->runAction(Sequence::create(FadeIn::create(0.0184),
                                            EaseOut::create(action, 2.0),
                                            remove, NULL));
    
}

void RhythmAdjustScene::onTouchesBegan(const std::vector<Touch *> &touches, cocos2d::Event *unused_event)
{
    
}

void RhythmAdjustScene::onTouchesEnded(const std::vector<Touch *> &touches, cocos2d::Event *unused_event)
{
    
}
