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
#include "json11.hpp"
#include <LLAudioEngine/LLAudioEngine.h>
#include "VideoManager.h"
#include "HomeScene.h"

#pragma mark Inline function

/**
 *  ある点がタップ可能な範囲内に入っているかを判定する
 *  判定方法はある点を基準にした半径r内に目的のSpriteが入っているかで判定する
 *  指の半径:    44px (iPhone4/4s上での指の大きさ)
 *  Sprite半径: 128px
 *
 *  @param pos タップした点
 *
 *  @return true: 半径が交差している, false: 判定ミス
 */
inline bool isPointContain(const Vec2& v1, const Vec2& v2)
{
    Circle *fingerCircle = Circle::create(v1, 44);
    Circle *targetCirlce = Circle::create(v2, 128);
    
    return targetCirlce->intersectCircle(fingerCircle);
    
}

#pragma mark - PlayScene implements

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

/**
 TODO: メソッド内部に処理を書きすぎている
 適宜処理をメソッドにして、処理を分ける必要がある
 */
bool PlayScene::init(std::string playSongFile, GameLevel gameLevel)
{
    if(!Layer::init())
    {
        return false;
    }

    //PLAYUI用のテクスチャアトラスを読み込む
    SpriteFrameCache::getInstance()->addSpriteFramesWithFile("res/PlayUI.plist");


    ValueMap fileInfo = FileUtils::getInstance()->getValueMapFromFile((playSongFile + "/fileInfo.plist").c_str());
    _songFilePath = playSongFile + '/' + fileInfo.at("BGM").asString();
    std::string songName = fileInfo["Name"].isNull() ? "" : fileInfo.at("Name").asString();
    std::string videoFilePath = fileInfo["BGV"].isNull() ? "" : fileInfo.at("BGV").asString();
    std::string coverImagePath = fileInfo["Cover"].isNull() ? "" : fileInfo.at("Cover").asString();
    
    std::string modeNameText;
    Sprite* backGroundImage;

    //ノーツの速度を設定(ms)
    // EASY | NORMAL | HARD | EXPERT | MASTER
    // 1600    1300    1000    800      600(未実装)
    switch (gameLevel)
    {
        case GameLevel::EASY:
            _notesSpeedMs = 1.6 * 1000.0f;
            modeNameText = "EASY";
            backGroundImage = Sprite::create("background/background_easy.png");
            break;
        case GameLevel::NORMAL:
            _notesSpeedMs = 1.3 * 1000.0f;
            modeNameText = "NORMAL";
            backGroundImage = Sprite::create("background/background_normal.png");
            break;
        case GameLevel::HARD:
            _notesSpeedMs = 1.0 * 1000.0f;
            modeNameText = "HARD";
            backGroundImage = Sprite::create("background/background_hard.png");
            break;
        case GameLevel::EXPERT:
            _notesSpeedMs =  0.8 * 1000.0f;
            modeNameText = "EXPERT";
            backGroundImage = Sprite::create("background/background_expert.png");
            break;
        default:
            break;
    }
    
    //レイテンシーの値を取得する
    _latencyMs = UserDefault::getInstance()->getFloatForKey("LATENCY", 0.0f);
 
/*===============UI関係の処理========================*/

    //画面サイズを取得
    auto screenSize = Director::getInstance()->getVisibleSize();

    if (videoFilePath != "")
    {
        _playVideoPath = playSongFile + '/' + videoFilePath;
    }

    LayerColor* blackLayer = LayerColor::create(Color4B::BLACK, screenSize.width, screenSize.height);
    blackLayer->setOpacity(40);
    blackLayer->setName("BlackLayer");
    this->addChild(blackLayer);

    //背景画像の設定
    backGroundImage->setName("backgroundImage");
    backGroundImage->setPosition(screenSize.width / 2, screenSize.height / 2);
    backGroundImage->setLocalZOrder(-1);
    this->addChild(backGroundImage);
    
    auto playScene = CSLoader::getInstance()->createNode("res/PlayScene.csb");
    playScene->setName("PlayLayer");
    playScene->setLocalZOrder(0);
    playScene->setOpacity(0);

    this->addChild(playScene);
    
    // ユニット画像の設定
    // オフスクリーンレンダリングで画像を作る
    makeUnitFrameAtlas();
    
    for (int i = 0; i < 9; i++)
    {
        auto sprite = Sprite::createWithSpriteFrameName(std::to_string(i)+".png");
        sprite->setPosition(SifUtil::unitPosition(i));
        sprite->getTexture()->setAliasTexParameters();
        playScene->addChild(sprite);
    }
    
/*===============譜面関係の処理========================*/

    
    //譜面情報を取得
    ValueMap modes = fileInfo.at("MODE").asValueMap();
    std::string jsonFileName = modes.at(modeNameText).asString();
    std::string jsonFileData = FileUtils::getInstance()->getStringFromFile(playSongFile + "/" + jsonFileName);
    std::string err;
    json11::Json jsonInfo = json11::Json::parse(jsonFileData, err);
    
    
    int notesCount = 0;
    _currentScore = 0;
    for(auto &lanes : jsonInfo["lane"].array_items())
    {
        std::deque<ValueMap> tmpNotesVec;
        
        for (auto &notesInfo : lanes.array_items())
        {
            ValueMap notes;
            
            notes["starttime"] = notesInfo["starttime"].number_value();
            notes["endtime"] = notesInfo["endtime"].number_value();
            notes["lane"] = notesInfo["lane"].number_value();
            
            notes["parallel"] = notesInfo["parallel"].bool_value();
            notes["hold"] = notesInfo["hold"].bool_value();
            notes["longnote"] = notesInfo["longnote"].bool_value();
            
            notes["latency"] = _latencyMs;
            notes["type"] = fileInfo["Type"].isNull() ? NoteType::SMILE : fileInfo.at("Type").asInt();
            notes["speed"] = _notesSpeedMs;
            
            notesCount++;
            if(notes["longnote"].asBool()) notesCount++;
            
            tmpNotesVec.push_back(std::move(notes));
        }
        _notesVector.push_back(tmpNotesVec);
    }
    
    //すべてパーフェクトを出したときのスコアを設定
    _maxScore = 100 * notesCount;

/*===============アニメーション関係の処理========================*/
    
    //ゲーム開始時のアニメーション用のレイヤーを重ねる
    auto playSplashLayer = CSLoader::getInstance()->createNode("res/splash_layer.csb");
    playSplashLayer->setName("SplashLayer");
    playSplashLayer->setLocalZOrder(1);
    auto jacketImage = playSplashLayer->getChildByName<Sprite*>("jacketImage");
    Sprite* sp = Sprite::create(playSongFile + '/' + coverImagePath);
    if(sp->getContentSize().width != jacketImage->getContentSize().width)
    {
        double scale_factor = jacketImage->getContentSize().width / sp->getContentSize().width;
        sp = Sprite::create(playSongFile + '/' + coverImagePath, scale_factor);
    }
    jacketImage->setTexture(sp->getTexture());
    
    auto songNameLabel = playSplashLayer->getChildByName<ui::Text*>("song_name");
    auto nameShadow = playSplashLayer->getChildByName<ui::Text*>("song_name_shadow");
    songNameLabel->setString(songName);
    nameShadow->setString(songName);
    playSplashLayer->setVisible(false);
    this->addChild(playSplashLayer);
    
    
    // 作成したノーツを格納するベクターを初期化
    // ここの9は9レーン分を挿入するために定義
    //_displayedNotes = std::vector< std::deque<Note*> >(9);
    _displayedNotes.resize(9);
    
    //タッチイベントリスナーを作成
    auto listener = cocos2d::EventListenerTouchAllAtOnce::create();
    listener->setEnabled(true);
    listener->onTouchesBegan = CC_CALLBACK_2(PlayScene::onTouchesBegan, this);
    listener->onTouchesMoved = CC_CALLBACK_2(PlayScene::onTouchesMoved, this);
    listener->onTouchesEnded = CC_CALLBACK_2(PlayScene::onTouchesEnded, this);
    this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, this);
    return true;
}

void PlayScene::onEnterTransitionDidFinish()
{
    auto playSplashLayer = getChildByName("SplashLayer");
    playSplashLayer->setVisible(true);
    
    //アニメーションを動かす
    auto action = cocostudio::timeline::ActionTimelineCache::getInstance()->createAction("res/splash_layer.csb");
    
    
    //最終フレームに到達したら、スプラッシュ画像を消去し、ゲーム開始準備を実行
    action->setLastFrameCallFunc([playSplashLayer,this]()
                                 {
                                     playSplashLayer->runAction(Sequence::create(FadeTo::create(1.0f, 0),
                                                                                 CallFunc::create(
                                                                                                  [this, playSplashLayer]()
                                                                                                  {
                                                                                                      this->removeChild(playSplashLayer);
                                                                                                      this->prepareGameRun();
                                                                                                  }),
                                                                                 NULL));
                                     
                                 });
    
    playSplashLayer->runAction(action);
    action->gotoFrameAndPlay(0, false);
}

#pragma mark Private methods


void PlayScene::prepareGameRun()
{
    auto playScene = this->getChildByName("PlayLayer");
    auto playSceneAction = cocostudio::timeline::ActionTimelineCache::getInstance()->createAction("res/PlayScene.csb");
    Sprite* musicNotes = playScene->getChildByName<Sprite*>("music_icon_7");
    musicNotes->runAction(playSceneAction);
    playSceneAction->gotoFrameAndPlay(0, true);
    
    playSceneAction = cocostudio::timeline::ActionTimelineCache::getInstance()->createAction("res/songCircle.csb");
    Layer* songCircleLayer = playScene->getChildByName<Layer*>("songCircle");
    songCircleLayer->runAction(playSceneAction);
    playSceneAction->gotoFrameAndPlay(0, true);
    
    //背景画像を設定
    if (_playVideoPath != "")
    {
        
        auto backSprite = this->getChildByName<Sprite*>("backgroundImage");
        backSprite->runAction(FadeTo::create(0.5f, 0));
    }
    
    auto blackBackLayer = this->getChildByName<LayerColor*>("BlackLayer");
    auto fadeOutAction =FadeTo::create(0.5f, _playVideoPath != "" ? 225 : 170);
    auto runGameAction = CallFunc::create([this, playScene](){
        playScene->setOpacity(255);
        this->run();
    });
    
    blackBackLayer->runAction(Sequence::create(fadeOutAction, runGameAction, NULL));

    LLAudioEngine::getInstance()->unloadAllEffect();
    //音声のプリロード
    std::string fullPath = FileUtils::getInstance()->fullPathForFilename("Sound/SE/perfect.mp3");
    LLAudioEngine::getInstance()->preloadEffect(fullPath);
    fullPath = FileUtils::getInstance()->fullPathForFilename("Sound/SE/great.mp3");
    LLAudioEngine::getInstance()->preloadEffect(fullPath);
    fullPath = FileUtils::getInstance()->fullPathForFilename("Sound/SE/good.mp3");
    LLAudioEngine::getInstance()->preloadEffect(fullPath);
    fullPath = FileUtils::getInstance()->fullPathForFilename("Sound/SE/bad.mp3");
    LLAudioEngine::getInstance()->preloadEffect(fullPath);
}

void PlayScene::run()
{

    std::function<void()> func = [this]()
    {
        //音楽の再生
        LLAudioEngine::getInstance()->playBackgroundMusic(_songFilePath);
        LLAudioEngine::getInstance()->setBackgroundExitCallback(CC_CALLBACK_0(PlayScene::finishCallBack, this));
        this->scheduleUpdate();
    };
    
    if(_playVideoPath != "")
    {
        //動画再生の開始
        VideoManager::play(_playVideoPath, func);
    }
    else
    {
        func();
    }
}

void PlayScene::update(float unused_dt)
{
    //Millisec単位で計測開始からの時間を取得
    double elapse = LLAudioEngine::getInstance()->tellBackgroundMusic();
    if (elapse <= 0)
    {
        elapse = 0.0;
    }
    //イテレータで_notes_vectorを最初から最後まで探索
    //そして、今の時間(ms)を超えたノーツが存在する場合、新しいノーツを生成する
    for (std::vector<std::deque<ValueMap>>::iterator it = _notesVector.begin(); it != _notesVector.end();)
    {
        if(it->front().at("starttime").asDouble() - _notesSpeedMs - _latencyMs < elapse)
        {
            createNotes(it->front());
            it->pop_front();
            
            if(it->empty())
            {
                it = _notesVector.erase(it);
                continue;
            }
        }
        it++;
    }
}


void PlayScene::createNotes(const ValueMap& map)
{
    Note* note = Note::create(map);

    //画面の判定外に出た場合の処理
    note->setOutDisplayedCallback(CC_CALLBACK_1(PlayScene::noteOutDisplayedCallback, this));

    //タップ判定を行った後の処理
    note->setTouchCallback(CC_CALLBACK_1(PlayScene::noteTouchCallback, this));

    //ロングノーツを離したときの処理
    note->setReleaseCallback(CC_CALLBACK_1(PlayScene::noteReleaseCallback, this));
    _displayedNotes[note->getLane()].emplace_back(note);
    
    addChild(note);
}

void PlayScene::createJudgeSprite(NoteJudge judge)
{
    std::string spritePath;
    switch (judge)
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
    Sprite* judgeSprite = Sprite::createWithSpriteFrameName(spritePath);
    judgeSprite->setName("JudgeSprite");
    auto rect = Director::getInstance()->getVisibleSize();
    judgeSprite->setPosition(rect.width / 2, rect.height / 2);
    judgeSprite->setScale(0.0f);
    judgeSprite->setOpacity(0);
    judgeSprite->setLocalZOrder(1);
    auto action1 = EaseSineOut::create(Spawn::create(ScaleTo::create(0.05f, 2.0f),
                                                     FadeTo::create(0.05f, 255), NULL));
    
    auto action2 = FadeOut::create(0.2f);
    addChild(judgeSprite);
    
    judgeSprite->runAction(Sequence::create(action1,
                                            DelayTime::create(0.15f),
                                            action2,
                                            RemoveSelf::create(), NULL));
    
}

void PlayScene::createTapFx(Vec2 position)
{
    auto tapFx= CSLoader::getInstance()->createNode("res/tapFx_0.csb");
    tapFx->setLocalZOrder(0);
    /*座標変換*/
    Vec2 rePosition = position;
    rePosition.x -= tapFx->getContentSize().width / 2;
    rePosition.y -= tapFx->getContentSize().height / 2;
    tapFx->setPosition(rePosition);
    this->addChild(tapFx);
    cocostudio::timeline::ActionTimeline* action = cocostudio::timeline::ActionTimelineCache::getInstance()->createAction("res/tapFx_0.csb");
    action->setLastFrameCallFunc([tapFx, this](){
        this->removeChild(tapFx);
    });
    
    tapFx->runAction(action);
    action->gotoFrameAndPlay(0, false);
}

#pragma mark Override functions

double sleeptime;

void PlayScene::applicationDidEnterBackground()
{
    LLAudioEngine::getInstance()->pauseBackgroundMusic();
    unscheduleUpdate();
    this->pause();
    VideoManager::pause();
}


void PlayScene::applicationWillEnterForeground()
{
    scheduleUpdate();
    float t = LLAudioEngine::getInstance()->tellBackgroundMusic();
    VideoManager::seekTo(t);
    VideoManager::resume();
    this->resume();

    LLAudioEngine::getInstance()->resumeBackgroundMusic();
}

void PlayScene::onTouchesBegan(const std::vector<Touch *> &touches, Event *event)
{
    for(Touch* touch : touches)
    {
        Vec2 location = touch->getLocation();
        
        // ひとつの指の位置に対し、複数のスプライトがタップ反応するとき
        // 押した場所から最も近いスプライトに対して処理を行う
        double minLength = DBL_MAX;
        int minLaneNum = -1;
        for (int i = 0;i < 9;i++)
        {
            if(_displayedNotes[i].empty()) continue;
            Vec2 position = SifUtil::unitPosition(i);
            if (!isPointContain(location, position))
            {
                continue;
            }
            double length = (location - position).length();
            
            if(length < minLength)
            {
                minLength = length;
                minLaneNum = i;
            }
        }
        if(minLaneNum == -1) continue;
        
        
        //min_lane_numのノーツに対してタップ処理をする
        bool result = _displayedNotes[minLaneNum].front()->touchBeginAction(touch->getID());
        if(result)
        {
            if (_displayedNotes[minLaneNum].front()->isLongNotes())
                _holdNotes[touch->getID()] = _displayedNotes[minLaneNum].front();
            _displayedNotes[minLaneNum].pop_front();
        }
    }
}

//
void PlayScene::onTouchesMoved(const std::vector<Touch *> &touches, Event *event)
{
    if(_holdNotes.empty()) return;
    
    for (Touch* touch : touches)
    {
        auto it = _holdNotes.find(touch->getID());
        if(it != _holdNotes.end() && it->second != nullptr)
        {
            int laneNum = it->second->getLane();
            Vec2 point = SifUtil::unitPosition(laneNum);
            if(isPointContain(touch->getLocation(), point)) continue;
            
            it->second->touchMoveAction(touch->getID());
            it->second = nullptr;
            _holdNotes.erase(it);
        }
    }
}

void PlayScene::onTouchesEnded(const std::vector<Touch *> &touches, Event *event)
{
    if(_holdNotes.empty()) return;
    
    for (Touch* touch : touches)
    {
        std::unordered_map<int, Note*>::iterator it = _holdNotes.find(touch->getID());
        if(it != _holdNotes.end() && it->second != nullptr)
        {
            it->second->touchEndAction(touch->getID());
            it->second = nullptr;
            _holdNotes.erase(it);
        }
    }
}

#pragma mark Callback functions

void PlayScene::finishCallBack()
{
    Director::getInstance()->getScheduler()->performFunctionInCocosThread([](){
        LLAudioEngine::getInstance()->unloadAllEffect();
        Director::getInstance()->purgeCachedData();
        VideoManager::stop();
        Scene* homeScene = HomeScene::createScene(ViewScene::Live);
        Director::getInstance()->replaceScene(TransitionFade::create(0.5f, homeScene, Color3B::BLACK));

    });
}

void PlayScene::noteOutDisplayedCallback(const Note& note)
{
    Sprite* judgeSprite = this->getChildByName<Sprite*>("JudgeSprite");
    if(judgeSprite != nullptr)
        removeChild(judgeSprite);
    auto overSprite = getChildByName<Sprite*>("OverPerfect");
    if(overSprite != nullptr) removeChild(overSprite);
    //Missの処理を行う
    createJudgeSprite(NoteJudge::MISS);
    
    this->_displayedNotes[note.getLane()].pop_front();
}

void PlayScene::noteTouchCallback(const Note& note)
{
    callbackHelperFunc(note);
}

void PlayScene::noteReleaseCallback(const Note& note)
{
    callbackHelperFunc(note, true);
}

void PlayScene::callbackHelperFunc(const Note& note, bool isRelease)
{
    Sprite* judgeSprite = this->getChildByName<Sprite*>("JudgeSprite");
    if(judgeSprite != nullptr)
        removeChild(judgeSprite);
    
    Sprite* overSprite = getChildByName<Sprite*>("OverPerfect");
    if(overSprite != nullptr)
        removeChild(overSprite);

    //タッチの判定により処理を変える
    std::string fullpath;
    try {
        switch (note.getJudgeResult())
        {
            case NoteJudge::PERFECT:
                fullpath=FileUtils::getInstance()->fullPathForFilename("Sound/SE/perfect.mp3");
                break;
            case NoteJudge::GREAT:
                fullpath=FileUtils::getInstance()->fullPathForFilename("Sound/SE/great.mp3");
                break;
            case NoteJudge::GOOD:
                fullpath=FileUtils::getInstance()->fullPathForFilename("Sound/SE/good.mp3");
                break;
            case NoteJudge::BAD:
                fullpath=FileUtils::getInstance()->fullPathForFilename("Sound/SE/bad.mp3");
                break;
            default:
                throw std::runtime_error("Invalid Judge");
                break;
        }
        
        //音の再生
        float soundTime = LLAudioEngine::getInstance()->tellBackgroundMusic();
        if (soundTime - _previousSoundTime > 20)
        {
            LLAudioEngine::getInstance()->playEffect(fullpath);
        }
        _previousSoundTime = soundTime;
    
    } catch (std::runtime_error e) {
        CCLOG(e.what());
    }

    //判定とタッチのエフェクトを表示する
    createJudgeSprite(note.getJudgeResult());
    //if(isRelease || !note.isLongNotes())
        //createTapFx(note.getChildByName<Sprite*>("BaseNotes")->getPosition());
}


void PlayScene::makeUnitFrameAtlas()
{
    constexpr int imgWidth = 128;
    constexpr int imgHeight = 128;
    int x = 0;
    int y = 0;
    ValueVector data = FileUtils::getInstance()->getValueVectorFromFile("UnitData.plist");
    
    // 縦横３つずつの画像が入る大きさを確保する
    RenderTexture* texture = RenderTexture::create(512, 512);
    texture->beginWithClear(0, 0, 0, 0);
    
    for (Value fileNameData : data)
    {
        auto sprite = Sprite::create(fileNameData.asString());
        sprite->setScaleY(-1.0);
        sprite->setAnchorPoint(Vec2(0,1));
        sprite->setPosition(Vec2(x, y));
        
        x += imgWidth;
        if(x >= 3 * imgWidth)
        {
            x = 0;
            y += imgHeight;
        }
        
        //書き込み
        sprite->visit();
        Director::getInstance()->getTextureCache()->removeTextureForKey(fileNameData.asString());
    }
    
    texture->end();
    
    //スプライトフレームを作成
    x = 0;
    y = 0;
    for(int i=0;i < 9;i++)
    {
        SpriteFrame* frame = SpriteFrame::createWithTexture(texture->getSprite()->getTexture(), Rect(x, y, imgWidth, imgHeight));
        SpriteFrameCache::getInstance()->addSpriteFrame(frame, std::to_string(i)+".png");
        x += imgWidth;
        if(x >= 3 * imgWidth)
        {
            x = 0;
            y += imgHeight;
        }
    }
}