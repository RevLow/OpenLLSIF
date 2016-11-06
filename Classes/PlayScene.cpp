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
#include "json11.hpp"
//#include "SimpleAudioEngine.h"
#include <LLAudioEngine/LLAudioEngine.h>
#include "HomeScene.h"

#pragma mark Inline function

/**
 *  ある点がタップ可能な範囲内に入っているかを判定する
 *  判定方法はある点を基準にした半径r内に目的のSpriteが入っているかで判定する
 *  指の半径:    44px (iPhone4/4s上での指の大きさ)
 *  Sprite半径: 64px
 *
 *  @param pos タップした点
 *
 *  @return true: 半径が交差している, false: 判定ミス
 */
inline bool isPointContain(const Vec2& v1, const Vec2& v2)
{
    Circle *fingerCircle = Circle::create(v1, 44);
    Circle *targetCirlce = Circle::create(v2, 64);
    
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

    
    //BGVが設定されている場合、ビデオを再生するためのレイヤーを追加
    //ただし、cocos2dのコードそのままだと最前面にビデオが来てしまうため
    //http://discuss.cocos2d-x.org/t/enhancement-request-for-videoplayer/16024
    //を参考にエンジン本体のコードを変更する
    if(videoFilePath != "")
    {
        auto videoPlayer = cocos2d::experimental::ui::VideoPlayer::create();
        videoPlayer->setContentSize(screenSize);
        videoPlayer->setPosition(Vec2(screenSize.width / 2, screenSize.height / 2));
        videoPlayer->setKeepAspectRatioEnabled(true);
        this->addChild(videoPlayer, -1);
        videoPlayer->setName("VideoLayer");
        videoPlayer->setFileName(playSongFile + '/' + videoFilePath);
        videoPlayer->prepareVideo(); //ビデオファイルをあらかじめロードしておくため、エンジンのビデオプレイヤーに処理を追加する
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
    this->addChild(playSplashLayer);
    
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
    
    
    // 作成したノーツを格納するベクターを初期化
    // ここの9は9レーン分を挿入するために定義
    _displayedNotes = std::vector< std::deque<Note*> >(9);
    
    //draw callを減らすためScoreLabelとlife_textのグローバルZを大きくし、別にレンダリングする
    for(std::string labelName : {"ScoreLabel", "life_text"})
    {
        ui::TextAtlas* atlasLabel = playScene->getChildByName<ui::TextAtlas *>(labelName);
        auto cloneLabel = atlasLabel->clone();
        cloneLabel->setGlobalZOrder(1);
        cloneLabel->setLocalZOrder(2);
        cloneLabel->setOpacity(0);
        addChild(cloneLabel);
        playScene->removeChild(atlasLabel);
    }
    
    //タッチイベントリスナーを作成
    auto listener = cocos2d::EventListenerTouchAllAtOnce::create();
    listener->setEnabled(true);
    listener->onTouchesBegan = CC_CALLBACK_2(PlayScene::onTouchesBegan, this);
    listener->onTouchesMoved = CC_CALLBACK_2(PlayScene::onTouchesMoved, this);
    listener->onTouchesEnded = CC_CALLBACK_2(PlayScene::onTouchesEnded, this);
    this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, this);
    std::function<void(void)> callbackFunc;
    return true;
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
    auto videoLayer = this->getChildByName<experimental::ui::VideoPlayer*>("VideoLayer");

    if (videoLayer != nullptr)
    {
        
        auto backSprite = this->getChildByName<LayerColor*>("backgroundImage");
        backSprite->runAction(FadeTo::create(0.5f, 0));
    }
    
    auto blackBackLayer = this->getChildByName<LayerColor*>("BlackLayer");
    auto fadeOutAction =FadeTo::create(0.5f, videoLayer != nullptr ? 225 : 170);
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
    auto videoLayer = this->getChildByName<experimental::ui::VideoPlayer*>("VideoLayer");
    //スコアとライフの透明度を変更
    this->getChildByName("ScoreLabel")->setOpacity(255);
    this->getChildByName("life_text")->setOpacity(255);
    
    
    //動画再生の開始
    if(videoLayer != nullptr) videoLayer->play();
    //音楽の再生
    LLAudioEngine::getInstance()->playBackgroundMusic(_songFilePath);
    LLAudioEngine::getInstance()->setBackgroundExitCallback(CC_CALLBACK_0(PlayScene::finishCallBack, this));
    this->scheduleUpdate();
}

void PlayScene::update(float unused_dt)
{
    //スコアの設定
    auto playSceneLayer = this->getChildByName("PlayLayer");
    auto score = this->getChildByName<ui::TextAtlas*>("ScoreLabel");
    
    score->setString(std::to_string(_currentScore));
    
    auto loadingBar = playSceneLayer->getChildByName<ui::LoadingBar*>("LoadingBar_1");
    loadingBar->setPercent(100.0f * ((double)_currentScore / (double)_maxScore) + 5.0f);
    
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
    Note *note = Note::create(map);

    //画面の判定外に出た場合の処理
    note->setOutDisplayedCallback(CC_CALLBACK_1(PlayScene::noteOutDisplayedCallback, this));

    //タップ判定を行った後の処理
    note->setTouchCallback(CC_CALLBACK_1(PlayScene::noteTouchCallback, this));

    //ロングノーツを離したときの処理
    note->setReleaseCallback(CC_CALLBACK_1(PlayScene::noteReleaseCallback, this));
    _displayedNotes[note->getLane()].push_back(note);
    
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
    judgeSprite->setScale(0.42f);
    judgeSprite->setOpacity(0);
    judgeSprite->setLocalZOrder(1);
    auto action1 = Spawn::create(ScaleTo::create(0.06f, 2.0f),
                                  FadeTo::create(0.06f, 255), NULL);
    
    auto action2 = FadeOut::create(0.3f);
    addChild(judgeSprite);
    
    judgeSprite->runAction(Sequence::create(action1,
                                             DelayTime::create(0.1f),
                                             action2,
                                             RemoveSelf::create(), NULL));
    
    //Perfectの場合は既存のPERFECTの画像の上にもう一枚上乗せする
    if(judge==NoteJudge::PERFECT)
    {
        Sprite *overSprite = Sprite::createWithSpriteFrameName("Image/Judge/judging_19.png");
        overSprite->setName("OverPerfect");
        overSprite->setPosition(judgeSprite->getPosition());
        overSprite->cocos2d::Node::setScale(0.5);
        overSprite->setLocalZOrder(1);
        overSprite->setOpacity(0);
        overSprite->setBlendFunc(BlendFunc::ADDITIVE);
        addChild(overSprite);
        
        overSprite->runAction(Sequence::create(FadeIn::create(0.06f),
                                               Spawn::create(FadeOut::create(0.1f),
                                                             ScaleTo::create(0.05f, 2.0f), NULL),
                                               RemoveSelf::create(),NULL));
    }
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

void PlayScene::applicationDidEnterBackground()
{
    LLAudioEngine::getInstance()->pauseBackgroundMusic();
    unscheduleUpdate();


    this->pause();
    auto videoPlayer =  this->getChildByName<experimental::ui::VideoPlayer*>("VideoLayer");
    videoPlayer->pause();
}


void PlayScene::applicationWillEnterForeground()
{
    scheduleUpdate();
    auto videoPlayer =  this->getChildByName<experimental::ui::VideoPlayer*>("VideoLayer");
    videoPlayer->resume();
    this->resume();

    LLAudioEngine::getInstance()->resumeBackgroundMusic();
}

void PlayScene::onTouchesBegan(const std::vector<Touch *> &touches, Event *event)
{
    Layer* playLayer = getChildByName<Layer*>("PlayLayer");
    
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
            int lane_i = i + 1;
            
            Sprite* sp = playLayer->getChildByName<Sprite*>(std::to_string(lane_i));

            if (!isPointContain(location, sp->getPosition()))
            {
                continue;
            }
            double length = (location - sp->getPosition()).length();
            
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
    
    Layer* playLayer = getChildByName<Layer*>("PlayLayer");
    for (Touch* touch : touches)
    {
        auto it = _holdNotes.find(touch->getID());
        if(it != _holdNotes.end() && it->second != nullptr)
        {
            int laneNum = it->second->getLane() + 1;
            Vec2 point = playLayer->getChildByName<Sprite*>(std::to_string(laneNum))->getPosition();
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
    if(isRelease || !note.isLongNotes())
        createTapFx(note.getChildByName<Sprite*>("BaseNotes")->getPosition());
}