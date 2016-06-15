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
#include "UIVideoPlayer.h"
#include <thread>
#include "json11.hpp"
#include "AudioManager.h"
#include "HomeScene.h"
#include "StopWatch.h"



Scene* PlayScene::createScene(std::string playSongFile, GameLevel level)
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();
    
    // 'layer' is an autorelease object
    auto layer = PlayScene::create(playSongFile, level);
    
    // add layer as a child to scene
    scene->addChild(layer);
    
    // return the scene
    return scene;
}

bool PlayScene::init(std::string playSongFile, GameLevel level)
{
    if(!Layer::init())
    {
        return false;
    }
    
    //PLAYUI用のテクスチャアトラスを読み込む
    SpriteFrameCache::getInstance()->addSpriteFramesWithFile("res/PlayUI.plist");


    ValueMap info = FileUtils::getInstance()->getValueMapFromFile((playSongFile + "/fileInfo.plist").c_str());
    songFilePath = playSongFile + '/' + info.at("BGM").asString();
    std::string songName = info["Name"].isNull() ? "" : info.at("Name").asString();
    std::string videoPath = info["BGV"].isNull() ? "" :info.at("BGV").asString();
    std::string coverImage = info["Cover"].isNull() ? "" : info.at("Cover").asString();
    
    std::string ModeText;
    Sprite* backgroundSprite;

    switch (level)
    {
        case GameLevel::EASY:
            notesSpeed = 1.6 * 1000.0f;
            ModeText = "EASY";
            backgroundSprite = Sprite::create("background/background_easy.png");
            break;
        case GameLevel::NORMAL:
            notesSpeed = 1.3 * 1000.0f;
            ModeText = "NORMAL";
            backgroundSprite = Sprite::create("background/background_normal.png");
            break;
        case GameLevel::HARD:
            notesSpeed = 1.0 * 1000.0f;
            ModeText = "HARD";
            backgroundSprite = Sprite::create("background/background_hard.png");
            break;
        case GameLevel::EXPERT:
            notesSpeed =  0.8 * 1000.0f;
            ModeText = "EXPERT";
            backgroundSprite = Sprite::create("background/background_expert.png");
            break;
        default:
            break;
    }
    
    //レイテンシーの値を取得する
    latency = UserDefault::getInstance()->getFloatForKey("LATENCY", 0.0f);
    
    //譜面情報を取得
    ValueMap modes = info.at("MODE").asValueMap();
    std::string jsonFileName = modes.at(ModeText).asString();
    CCLOG("%s/%s", playSongFile.c_str(), jsonFileName.c_str());
    std::string fileData = FileUtils::getInstance()->getStringFromFile(playSongFile + "/" + jsonFileName);
    std::string err;
    json11::Json json = json11::Json::parse(fileData, err);
    
    
    note_count = 0;
    current_score = 0;
    for(auto &lanes : json["lane"].array_items())
    {
        std::shared_ptr< std::queue<std::shared_ptr<cocos2d::ValueMap>>> tmpVec=std::make_shared<std::queue<std::shared_ptr<cocos2d::ValueMap>>>();
        
        for (auto &laneItems : lanes.array_items())
        {
            note_count++;
            std::shared_ptr<cocos2d::ValueMap> notes(new cocos2d::ValueMap());
            (*notes)["starttime"]=laneItems["starttime"].number_value();
            (*notes)["endtime"]=laneItems["endtime"].number_value();
            (*notes)["parallel"]=laneItems["parallel"].bool_value();
            (*notes)["hold"]=laneItems["hold"].bool_value();
            (*notes)["lane"]=laneItems["lane"].number_value();
            (*notes)["longnote"]=laneItems["longnote"].bool_value();
            if((*notes)["endtime"].asDouble() >= BGM_TIME) BGM_TIME = (*notes)["endtime"].asDouble();
            (*notes)["latency"] = latency;
            
            //ノーツのタイプを取得、
            (*notes)["type"] = info["Type"].isNull()? 0 : info.at("Type").asInt();
            (*notes)["speed"]=notesSpeed;
            //CCLOG("%ld",notes.use_count());
            tmpVec->push(notes);
            
            //ロングノーツの場合、終点の分も数える
            if((*notes)["longnote"].asBool()) note_count++;
        }
        notesVector.push_back(tmpVec);
    }
    
    //すべてパーフェクトを出したときのスコアを設定
    max_score = 100 * note_count;
    
    //画面サイズを取得
    auto visibleSize = Director::getInstance()->getVisibleSize();

    
    //BGVが設定されている場合、ビデオを再生するためのレイヤーを追加
    //ただし、cocos2dのコードそのままだと最前面にビデオが来てしまうため
    //http://discuss.cocos2d-x.org/t/enhancement-request-for-videoplayer/16024
    //を参考にコードを変更する
    if(videoPath != "")
    {
        auto vidPlayer = cocos2d::experimental::ui::VideoPlayer::create();
        vidPlayer->setContentSize(visibleSize);
        vidPlayer->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
        vidPlayer->setKeepAspectRatioEnabled(true);
        this->addChild(vidPlayer, -1);
        vidPlayer->setName("VideoLayer");
        vidPlayer->setFileName(playSongFile + '/' + videoPath);
    }
    
    LayerColor *layer = LayerColor::create(Color4B::BLACK, visibleSize.width, visibleSize.height);
    layer->setOpacity(0);
    layer->setName("BlackLayer");
    this->addChild(layer);

    //背景画像の設定
    backgroundSprite->setName("backgroundImage");
    backgroundSprite->setPosition(visibleSize.width / 2, visibleSize.height / 2);
    backgroundSprite->setLocalZOrder(-1);
    this->addChild(backgroundSprite);
    
    auto playScene = CSLoader::getInstance()->createNode("res/PlayScene.csb");
    playScene->setName("PlayLayer");
    playScene->setLocalZOrder(0);
    playScene->setOpacity(0);
    this->addChild(playScene);

    
    auto startPoint = playScene->getChildByName<Sprite*>("music_icon_7");

    
    //SpriteFrameCache::getInstance()->addSpriteFramesWithFile("res/PlayUI.plist");
    for(int i=1;i<=9;i++)
    {
        std::stringstream ss;
        ss << i;
        auto targetPoint = playScene->getChildByName<Sprite*>(ss.str());
        cocos2d::Vec2 v = targetPoint->getPosition() - startPoint->getPosition();
      
        
        unitVector.push_back(v);
    }
    
    //ゲーム開始時のアニメーション用のレイヤーを重ねる
    auto playSplash = CSLoader::getInstance()->createNode("res/splash_layer.csb");
    playSplash->setLocalZOrder(1);
    auto spriteImage = playSplash->getChildByName<Sprite*>("jacketImage");
    Sprite* sp = Sprite::create(playSongFile + '/' + coverImage);
    if(sp->getContentSize().width != spriteImage->getContentSize().width)
    {
        double scalefactor = spriteImage->getContentSize().width / sp->getContentSize().width;
        sp = Sprite::create(playSongFile + '/' + coverImage, scalefactor);
    }
    spriteImage->setTexture(sp->getTexture());
    
    auto songNameLabel = playSplash->getChildByName<ui::Text*>("song_name");
    auto songNameShadowLabel = playSplash->getChildByName<ui::Text*>("song_name_shadow");
    songNameLabel->setString(songName);
    songNameShadowLabel->setString(songName);
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
    
    _longNotes = cocos2d::Map<int, Note*>();
    
    //Note型を格納するためのレイヤー
    Layer *notesLayer = Layer::create();
    notesLayer->setName("Notes_Layer");
    addChild(notesLayer);
    
    createdNotes.reserve(9);
    for (int i=0; i<9; ++i) {
        std::queue<Note*> notesQueue;
        createdNotes.push_back(notesQueue);
    }
    
        
    //draw callを減らすためScoreLabelとlife_textのグローバルZを大きくし、別にレンダリングする
    //playScene->removeChildByName("ScoreLabel");
    //playScene->removeChildByName("life_text");
    ui::TextAtlas* scoreLabel = playScene->getChildByName<ui::TextAtlas *>("ScoreLabel");
    auto cloneLabel = scoreLabel->clone();
    cloneLabel->setGlobalZOrder(1);
    cloneLabel->setLocalZOrder(2);
    cloneLabel->setOpacity(0);
    addChild(cloneLabel);
    
    playScene->removeChild(scoreLabel);
    
    
    ui::TextAtlas* life = playScene->getChildByName<ui::TextAtlas *>("life_text");
    auto clonelife = life->clone();
    clonelife->setGlobalZOrder(1);
    clonelife->setLocalZOrder(2);
    clonelife->setOpacity(0);
    addChild(clonelife);
    
    playScene->removeChild(life);

    
    return true;
}

/**
 * =========================================== Private Member Methods
 *
 */


/*
ゲームを開始する
 */
void PlayScene::Run()
{
    auto playScene = this->getChildByName("PlayLayer");
    auto action = cocostudio::timeline::ActionTimelineCache::getInstance()->createAction("res/PlayScene.csb");
    Sprite* music_notes = playScene->getChildByName<Sprite*>("music_icon_7");
    music_notes->runAction(action);
    action->gotoFrameAndPlay(0, true);
    
    action = cocostudio::timeline::ActionTimelineCache::getInstance()->createAction("res/songCircle.csb");
    Layer* songCircleLayer = playScene->getChildByName<Layer*>("songCircle");
    songCircleLayer->runAction(action);
    action->gotoFrameAndPlay(0, true);
    
    //背景画像を設定
    
    Vec2 v = unitVector[0];
    auto plist = FileUtils::getInstance()->getValueVectorFromFile("TapAreaList.plist");
    
    for (int i=0; i < plist.size(); i++)
    {
        auto lane = plist[i].asValueVector();
        Vector<Circle*> areas;
        for (int j=0; j <lane.size(); j++)
        {
            auto circleData = lane[j].asValueMap();
            
            float x = circleData["X"].asFloat();
            float y = circleData["Y"].asFloat();
            float r = circleData["R"].asFloat();
            
            Circle* c = Circle::create(Point(x, y), r);
            areas.pushBack(c);
        }
        expandedAreas.push_back(areas);
    }
    
    auto videoLayer = this->getChildByName<experimental::ui::VideoPlayer*>("VideoLayer");

    if (videoLayer != nullptr)
    {
        
        auto backSprite = this->getChildByName<LayerColor*>("backgroundImage");
        backSprite->runAction(FadeTo::create(0.5f, 0));
    }
    
    auto backgroundLayer = this->getChildByName<LayerColor*>("BlackLayer");
    backgroundLayer->runAction(FadeTo::create(0.5f, videoLayer!= nullptr ? 200 : 100));
    
    //音声のプリロード
    AudioManager::getInstance()->preload("Sound/SE/perfect.mp3");
    AudioManager::getInstance()->preload("Sound/SE/great.mp3");
    AudioManager::getInstance()->preload("Sound/SE/good.mp3");
    AudioManager::getInstance()->preload("Sound/SE/bad.mp3");
    //透明度を戻した後、実行を行う
    playScene->runAction(Sequence::create(
                                          FadeTo::create(0.5f, 255),
                                          CallFunc::create([this, videoLayer]()
                                                        {
                                                            //タッチイベントリスナーを作成
                                                            auto listener = cocos2d::EventListenerTouchAllAtOnce::create();
                                                            listener->setEnabled(true);
                                                            listener->onTouchesBegan = CC_CALLBACK_2(PlayScene::onTouchesBegan, this);
                                                            listener->onTouchesEnded = CC_CALLBACK_2(PlayScene::onTouchesEnded, this);
                                                            this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, this);
                                                            
                                                            //スコアとライフの透明度を変更
                                                            this->getChildByName("ScoreLabel")->setOpacity(255);
                                                            this->getChildByName("life_text")->setOpacity(255);
                                                            
                                                            //動画再生の開始
                                                            if(videoLayer != nullptr)
                                                                videoLayer->play();
                                                            
                                                            //音楽の再生
                                                            AudioManager::getInstance()->play(songFilePath,AudioManager::BGM);
                                                            AudioManager::getInstance()->setOnExitCallback(CC_CALLBACK_2(PlayScene::finishCallBack, this));
                                                            //StopWatchを稼働
                                                            StopWatch::getInstance()->start();
                                                            this->scheduleUpdate();
                                                        })
                                          , NULL));
}


void PlayScene::CreateNotes(std::vector< std::shared_ptr<cocos2d::ValueMap> > maps)
{
    Layer *notesLayer = getChildByName<Layer*>("Notes_Layer");
    
    for(auto note : maps)
    {
        Vec2 v = unitVector[note->at("lane").asInt()];
        
        //ちょっと長いけど対象の方向を
        std::string numStr = std::to_string(note->at("lane").asInt() + 1);
        Sprite* destinationSprite = this->getChildByName("PlayLayer")->getChildByName<Sprite*>(numStr);
        (*note)["destinationX"] = destinationSprite->getPosition().x;
        (*note)["destinationY"] = destinationSprite->getPosition().y;
        
        Note *n = Note::create(*note, v);
        createdNotes[note->at("lane").asInt()].push(n);
        
        //画面の判定外に出た場合の処理
        n->setOutDisplayedCallback([this, note]()
        {
            
            Sprite *jSprite = this->getChildByName<Sprite*>("JudgeSprite");
            if(jSprite != nullptr)
                removeChild(jSprite);
            auto overSprite = getChildByName<Sprite*>("OverPerfect");
            if(overSprite != nullptr) removeChild(overSprite);
            //Missの処理を行う
            this->CreateJudgeSprite(NoteJudge::MISS);
            createdNotes[note->at("lane").asInt()].pop();
        });
        
        notesLayer->addChild(n);
    }
}

void PlayScene::CreateJudgeSprite(NoteJudge j)
{
    Sprite *jSprite = this->getChildByName<Sprite*>("JudgeSprite");
    if(jSprite != nullptr)
        removeChild(jSprite);
    auto overSprite = getChildByName<Sprite*>("OverPerfect");
    if(overSprite != nullptr) removeChild(overSprite);

    //SpriteFrameCache::getInstance()->addSpriteFramesWithFile("res/JudgeAtlas.plist");

    std::string spritePath;
    switch (j)
    {
        case NoteJudge::PERFECT:
            spritePath = "Image/Judge/judging_15.png";
            break;
        case NoteJudge::GREAT:
            spritePath = "Image/Judge/judging_09.png";
            break;
        case NoteJudge::GOOD:
            spritePath = "Image/Judge/judging_17.png";
            break;
        case NoteJudge::BAD:
            spritePath = "Image/Judge/judging_07.png";
            break;
        case NoteJudge::MISS:
            spritePath = "Image/Judge/judging_08.png";
            break;
        default:
            break;
    }
    
    jSprite = Sprite::createWithSpriteFrameName(spritePath);
    jSprite->setName("JudgeSprite");
    
    auto rect = Director::getInstance()->getVisibleSize();
    jSprite->setPosition(rect.width / 2, rect.height / 2);
    jSprite->setScale(0.42f);
    jSprite->setOpacity(0);
    jSprite->setLocalZOrder(1);
    auto *action1 = Spawn::create(ScaleTo::create(0.06f, 2.0f), FadeTo::create(0.06f, 255), NULL);
    auto *action2 = FadeOut::create(0.3f);
    addChild(jSprite);
    
    jSprite->runAction(Sequence::create(action1, DelayTime::create(0.1f),action2, NULL));
    
    
    //Perfectの場合はオーバーする
    if(j==NoteJudge::PERFECT)
    {
        overSprite = Sprite::createWithSpriteFrameName("Image/Judge/judging_19.png");
        overSprite->setName("OverPerfect");
        overSprite->setPosition(jSprite->getPosition());
        overSprite->cocos2d::Node::setScale(0.5);
        overSprite->setLocalZOrder(1);
        overSprite->setOpacity(0);
        overSprite->setBlendFunc(BlendFunc::ADDITIVE);
        addChild(overSprite);
        
        overSprite->runAction(Sequence::create(FadeIn::create(0.06f),
                              Spawn::create(FadeOut::create(0.1f), ScaleTo::create(0.05f, 2.0f), NULL), NULL));
    }

    
}

void PlayScene::CreateTapFx(Vec2 position)
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

/**
 * =========================================== CallBacks
 *
 */


/*
 音楽再生終了時のコールバック関数
 */
void PlayScene::finishCallBack(int audioId, std::string fileName)
{
    CCLOG("FINISH");
    Director::getInstance()->purgeCachedData();
    
    Scene* scene = HomeScene::createScene(ViewScene::Live);
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene, Color3B::BLACK));
    StopWatch::getInstance()->stop();
}


/**
 * =========================================== Event and Delegate Methods
 *
 */


void PlayScene::applicationDidEnterBackground()
{
    unscheduleUpdate();
    StopWatch::getInstance()->pause();

    AudioManager::getInstance()->pause(AudioManager::BGM);
    this->pause();
    //Director::getInstance()->pause();
    auto videoLayer =  this->getChildByName<experimental::ui::VideoPlayer*>("VideoLayer");
    videoLayer->pause();

}


/*
 バックグラウンドから復帰したときに呼ぶ関数
 AppDelegateで呼ぶように指定してある
 */
void PlayScene::applicationWillEnterForeground()
{
 //   Director::getInstance()->resume();
    auto videoLayer =  this->getChildByName<experimental::ui::VideoPlayer*>("VideoLayer");
    videoLayer->resume();
    
    this->resume();
    AudioManager::getInstance()->resume(AudioManager::BGM);
    StopWatch::getInstance()->resume();
    scheduleUpdate();
}

void PlayScene::update(float dt)
{
    //スコアの設定
    auto playScene = this->getChildByName("PlayLayer");
    auto *score = this->getChildByName<ui::TextAtlas*>("ScoreLabel");
    std::stringstream ss;
    ss << current_score;

    score->setString(ss.str());
    
    auto loadingBar= playScene->getChildByName<ui::LoadingBar*>("LoadingBar_1");
    loadingBar->setPercent(100.0f * ((double)current_score / (double)max_score) + 5.0f);
    
    
    //Millisec単位で計測開始からの時間を取得
    double elapse = StopWatch::getInstance()->currentTime();
    
    //ここで指定の時間を超えた場合メインスレッド上でノートを作成し、
    //addChildさせる。
    std::vector< std::shared_ptr<cocos2d::ValueMap> > notes;
    for (auto it = notesVector.begin(); it != notesVector.end();)
    {
        //先頭の情報をとってくる
        //if(*it == nullptr) break;
        
        std::shared_ptr<cocos2d::ValueMap> map = (**it).front();
        if(map->at("starttime").asDouble() - notesSpeed - latency < elapse)
        {
            //CCLOG("Disparity: %lf", elapsed - map->at("starttime").asDouble());
            notes.push_back(map);
            (**it).pop();
            
            if((**it).empty())
            {
                it = notesVector.erase(it);
                continue;
            }
        }
        it++;
    }
    
    if(!notes.empty())
    {
        CreateNotes(notes);
    }
}

void PlayScene::onTouchesBegan(const std::vector<Touch *> &touches, cocos2d::Event *unused_event)
{
    
    //auto playScene = this->getChildByName("PlayLayer");
    //auto target = unused_event->getCurrentTarget();
    Layer *notesLayer = getChildByName<Layer*>("Notes_Layer");
    Vector<Node*> children = notesLayer->getChildren();
    
    //クリックされた位置を取得
    for(int i=0;i<9;i++)
    {
        for (int j=0; j < touches.size(); j++)
        {
            auto loc = touches[j]->getLocation();
            for(int k=0;k < expandedAreas[i].size();k++)
            {
                Circle *area = expandedAreas[i].at(k);
                if(area->containsPoint(loc))
                {
                    if(createdNotes[i].empty()) goto for_exit;
                    
                    Note* note = createdNotes[i].front();
                    NoteJudge judge = note->StartJudge();
                    
                    //もしもタップ可能区間に入っていないなら
                    if(judge == NoteJudge::NON)
                    {
                        //別のレーンの探索に行く
                        goto for_exit;
                    }
                    else
                    {
                        if(note->isLongNotes())
                        {
                            _longNotes.insert(touches[j]->getID(), note);
                        }
                        
                        std::string fullpath;
                        switch (judge)
                        {
                            case NoteJudge::PERFECT:
                                fullpath=FileUtils::getInstance()->fullPathForFilename("Sound/SE/perfect.mp3");
                                current_score += 100;
                                break;
                            case NoteJudge::GREAT:
                                fullpath=FileUtils::getInstance()->fullPathForFilename("Sound/SE/great.mp3");
                                current_score += 50;
                                break;
                            case NoteJudge::GOOD:
                                fullpath=FileUtils::getInstance()->fullPathForFilename("Sound/SE/good.mp3");
                                current_score += 10;
                                break;
                            case NoteJudge::BAD:
                                fullpath=FileUtils::getInstance()->fullPathForFilename("Sound/SE/bad.mp3");
                                current_score += 5;
                                break;
                            default:
                                break;
                        }
                        AudioManager::getInstance()->play(fullpath, AudioManager::SE);
                        CreateJudgeSprite(judge);
                        CreateTapFx(note->getChildByName<RenderTexture*>("BaseNotes")->getPosition());
                        
                        //先頭を取り出す
                        createdNotes[i].pop();
                        
                        //今のレーンの判定はこれ以上行わない
                        goto for_exit;
                    }

                }
            }
        }
for_exit:
        continue;
    }
    
}
void PlayScene::onTouchesEnded(const std::vector<Touch *> &touches, cocos2d::Event *unused_event)
{
        for(Touch* t : touches)
        {
            Note* n = _longNotes.at(t->getID());
            if(n != nullptr)
            {
                NoteJudge j = n->EndJudge();
                
                std::string fullpath;
                switch (j)
                {
                    case NoteJudge::PERFECT:
                        fullpath=FileUtils::getInstance()->fullPathForFilename("Sound/SE/perfect.mp3");
                        current_score += 100;
                        break;
                    case NoteJudge::GREAT:
                        fullpath=FileUtils::getInstance()->fullPathForFilename("Sound/SE/great.mp3");
                        current_score += 50;
                        break;
                    case NoteJudge::GOOD:
                        fullpath=FileUtils::getInstance()->fullPathForFilename("Sound/SE/good.mp3");
                        current_score += 10;
                        break;
                    case NoteJudge::BAD:
                        fullpath=FileUtils::getInstance()->fullPathForFilename("Sound/SE/bad.mp3");
                        current_score += 5;
                        break;
                    default:
                        break;
                }
                AudioManager::getInstance()->play(fullpath, AudioManager::SE);
                CreateJudgeSprite(j);
                CreateTapFx(n->getChildByName("BaseNotes")->getPosition());
                
                _longNotes.erase(t->getID());
            }
        }
}



