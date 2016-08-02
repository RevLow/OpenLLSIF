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

/**
 TODO:
 メソッド内部に処理を書きすぎている
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


    ValueMap file_info = FileUtils::getInstance()->getValueMapFromFile((playSongFile + "/fileInfo.plist").c_str());
    _song_file_path = playSongFile + '/' + file_info.at("BGM").asString();
    std::string song_name = file_info["Name"].isNull() ? "" : file_info.at("Name").asString();
    std::string video_file_path = file_info["BGV"].isNull() ? "" : file_info.at("BGV").asString();
    std::string cover_image_path = file_info["Cover"].isNull() ? "" : file_info.at("Cover").asString();
    
    std::string mode_name_text;
    Sprite*back_ground_image;

    //ノーツの速度を設定(ms)
    // EASY | NORMAL | HARD | EXPERT | MASTER
    // 1600    1300    1000    800      未実装
    switch (gameLevel)
    {
        case GameLevel::EASY:
            _notes_speed_ms = 1.6 * 1000.0f;
            mode_name_text = "EASY";
            back_ground_image = Sprite::create("background/background_easy.png");
            break;
        case GameLevel::NORMAL:
            _notes_speed_ms = 1.3 * 1000.0f;
            mode_name_text = "NORMAL";
            back_ground_image = Sprite::create("background/background_normal.png");
            break;
        case GameLevel::HARD:
            _notes_speed_ms = 1.0 * 1000.0f;
            mode_name_text = "HARD";
            back_ground_image = Sprite::create("background/background_hard.png");
            break;
        case GameLevel::EXPERT:
            _notes_speed_ms =  0.8 * 1000.0f;
            mode_name_text = "EXPERT";
            back_ground_image = Sprite::create("background/background_expert.png");
            break;
        default:
            break;
    }
    
    //レイテンシーの値を取得する
    _latency_ms = UserDefault::getInstance()->getFloatForKey("LATENCY", 0.0f);
 
/*===============UI関係の処理========================*/

    //画面サイズを取得
    auto screen_size = Director::getInstance()->getVisibleSize();

    
    //BGVが設定されている場合、ビデオを再生するためのレイヤーを追加
    //ただし、cocos2dのコードそのままだと最前面にビデオが来てしまうため
    //http://discuss.cocos2d-x.org/t/enhancement-request-for-videoplayer/16024
    //を参考にコードを変更する
    if(video_file_path != "")
    {
        auto video_player = cocos2d::experimental::ui::VideoPlayer::create();
        video_player->setContentSize(screen_size);
        video_player->setPosition(Vec2(screen_size.width / 2, screen_size.height / 2));
        video_player->setKeepAspectRatioEnabled(true);
        this->addChild(video_player, -1);
        video_player->setName("VideoLayer");
        video_player->setFileName(playSongFile + '/' + video_file_path);
        video_player->prepareVideo();
    }
    
    LayerColor *black_layer = LayerColor::create(Color4B::BLACK, screen_size.width, screen_size.height);
    black_layer->setOpacity(0);
    black_layer->setName("BlackLayer");
    this->addChild(black_layer);

    //背景画像の設定
    back_ground_image->setName("backgroundImage");
    back_ground_image->setPosition(screen_size.width / 2, screen_size.height / 2);
    back_ground_image->setLocalZOrder(-1);
    this->addChild(back_ground_image);
    
    auto play_scene = CSLoader::getInstance()->createNode("res/PlayScene.csb");
    play_scene->setName("PlayLayer");
    play_scene->setLocalZOrder(0);
    play_scene->setOpacity(0);
    this->addChild(play_scene);

    
    auto start_position = play_scene->getChildByName<Sprite*>("music_icon_7");


    //PlaySceneからそれぞれのボタンの位置を取得して、ノーツが進む方向を算出
    for(int i=1;i<=9;i++)
    {
        auto destination_position = play_scene->getChildByName<Sprite*>(std::to_string(i));
        cocos2d::Vec2 v = destination_position->getPosition() - start_position->getPosition();
      
        
        _direction_unit_vector.push_back(v);
    }
    
/*===============譜面関係の処理========================*/

    
    //譜面情報を取得
    ValueMap modes = file_info.at("MODE").asValueMap();
    std::string json_file_name = modes.at(mode_name_text).asString();
    std::string json_file_data = FileUtils::getInstance()->getStringFromFile(playSongFile + "/" + json_file_name);
    std::string err;
    json11::Json json_info = json11::Json::parse(json_file_data, err);
    
    
    int notes_count = 0;
    _current_score = 0;
    for(auto &lanes : json_info["lane"].array_items())
    {
        std::queue<ValueMapPtr> tmp_notes_vec;
        
        for (auto &notes_info : lanes.array_items())
        {
            notes_count++;
            ValueMapPtr notes(new ValueMap);
            (*notes)["starttime"]= notes_info["starttime"].number_value();
            (*notes)["endtime"]= notes_info["endtime"].number_value();
            (*notes)["parallel"]= notes_info["parallel"].bool_value();
            (*notes)["hold"]= notes_info["hold"].bool_value();
            (*notes)["lane"]= notes_info["lane"].number_value();
            (*notes)["longnote"]= notes_info["longnote"].bool_value();
            (*notes)["latency"] = _latency_ms;
            
            //ノーツのタイプを取得、
            (*notes)["type"] = file_info["Type"].isNull()? 0 : file_info.at("Type").asInt();
            (*notes)["speed"]=_notes_speed_ms;
            
            //ノーツが最終的に到達すべき位置を取得する
            std::string laneStr = std::to_string(notes_info["lane"].int_value()+1);
            Sprite* destinationSprite =play_scene->getChildByName<Sprite*>(laneStr);
            (*notes)["destinationX"] = destinationSprite->getPosition().x;
            (*notes)["destinationY"] = destinationSprite->getPosition().y;
            
            //ロングノーツの場合、終点の分も数える
            if((*notes)["longnote"].asBool()) notes_count++;
            
            tmp_notes_vec.push(notes);
        }
        _notes_vector.push_back(tmp_notes_vec);
    }
    
    //すべてパーフェクトを出したときのスコアを設定
    _max_score = 100 * notes_count;

/*===============アニメーション関係の処理========================*/
    
    //ゲーム開始時のアニメーション用のレイヤーを重ねる
    auto play_splash_layer = CSLoader::getInstance()->createNode("res/splash_layer.csb");
    play_splash_layer->setLocalZOrder(1);
    auto jacket_image = play_splash_layer->getChildByName<Sprite*>("jacketImage");
    Sprite* sp = Sprite::create(playSongFile + '/' + cover_image_path);
    if(sp->getContentSize().width != jacket_image->getContentSize().width)
    {
        double scale_factor = jacket_image->getContentSize().width / sp->getContentSize().width;
        sp = Sprite::create(playSongFile + '/' + cover_image_path, scale_factor);
    }
    jacket_image->setTexture(sp->getTexture());
    
    auto song_name_label = play_splash_layer->getChildByName<ui::Text*>("song_name");
    auto song_name_shadow = play_splash_layer->getChildByName<ui::Text*>("song_name_shadow");
    song_name_label->setString(song_name);
    song_name_shadow->setString(song_name);
    this->addChild(play_splash_layer);
    
    //アニメーションを動かす
    auto action = cocostudio::timeline::ActionTimelineCache::getInstance()->createAction("res/splash_layer.csb");
    
    
    //最終フレームに到達したら、スプラッシュ画像を消去し、ゲーム開始準備を実行
    action->setLastFrameCallFunc([play_splash_layer,this]()
                                {
                                    play_splash_layer->runAction(Sequence::create(FadeTo::create(1.0f, 0),
                                                                                   CallFunc::create(
                                                                                   [this, play_splash_layer]()
                                                                                   {
                                                                                       this->removeChild(play_splash_layer);
                                                                                       this->prepareGameRun();
                                                                                   }),
                                                                                   NULL));
    
                                });
    
    play_splash_layer->runAction(action);
    action->gotoFrameAndPlay(0, false);
    
    
    // 作成したノーツを格納するベクターを初期化
    // ここの9は9レーン分を挿入するために定義
    _displayed_notes = std::vector< std::queue<Note*> >(9);
    
    //draw callを減らすためScoreLabelとlife_textのグローバルZを大きくし、別にレンダリングする
    for(std::string label_name : {"ScoreLabel", "life_text"})
    {
        ui::TextAtlas* atlas_label = play_scene->getChildByName<ui::TextAtlas *>(label_name);
        auto clone_label = atlas_label->clone();
        clone_label->setGlobalZOrder(1);
        clone_label->setLocalZOrder(2);
        clone_label->setOpacity(0);
        addChild(clone_label);
        play_scene->removeChild(atlas_label);
    }
    
    return true;
}

/**
 * =========================================== Private Member Methods============================================
 *
 */


void PlayScene::prepareGameRun()
{
    auto play_scene = this->getChildByName("PlayLayer");
    auto play_scene_action = cocostudio::timeline::ActionTimelineCache::getInstance()->createAction("res/PlayScene.csb");
    Sprite* music_notes = play_scene->getChildByName<Sprite*>("music_icon_7");
    music_notes->runAction(play_scene_action);
    play_scene_action->gotoFrameAndPlay(0, true);
    
    play_scene_action = cocostudio::timeline::ActionTimelineCache::getInstance()->createAction("res/songCircle.csb");
    Layer*song_circle_layer = play_scene->getChildByName<Layer*>("songCircle");
    song_circle_layer->runAction(play_scene_action);
    play_scene_action->gotoFrameAndPlay(0, true);
    
    //背景画像を設定
    auto video_layer = this->getChildByName<experimental::ui::VideoPlayer*>("VideoLayer");

    if (video_layer != nullptr)
    {
        
        auto back_sprite = this->getChildByName<LayerColor*>("backgroundImage");
        back_sprite->runAction(FadeTo::create(0.5f, 0));
    }
    
    auto black_back_layer = this->getChildByName<LayerColor*>("BlackLayer");
    black_back_layer->runAction(FadeTo::create(0.5f,
                                               video_layer != nullptr ? 200 : 100));
    
    //音声のプリロード
    AudioManager::getInstance()->preload("Sound/SE/perfect.mp3");
    AudioManager::getInstance()->preload("Sound/SE/great.mp3");
    AudioManager::getInstance()->preload("Sound/SE/good.mp3");
    AudioManager::getInstance()->preload("Sound/SE/bad.mp3");
    
    //透明度を戻した後、実行を行う
    play_scene->runAction(Sequence::create(FadeTo::create(0.5f, 255),
                                           CallFunc::create(CC_CALLBACK_0(PlayScene::run, this)), NULL));
}

void PlayScene::run()
{
    auto video_layer = this->getChildByName<experimental::ui::VideoPlayer*>("VideoLayer");
    //スコアとライフの透明度を変更
    this->getChildByName("ScoreLabel")->setOpacity(255);
    this->getChildByName("life_text")->setOpacity(255);
    
    
    //動画再生の開始
    video_layer->play();
    //音楽の再生
    AudioManager::getInstance()->play(_song_file_path,AudioManager::BGM);
    AudioManager::getInstance()->setOnExitCallback(CC_CALLBACK_2(PlayScene::finishCallBack, this));
    StopWatch::getInstance()->start();
    this->scheduleUpdate();
}

void PlayScene::update(float dt)
{
    if(StopWatch::getInstance()->getStatus() != PLAYING)
    {
        return;
    }
    
    //スコアの設定
    auto play_scene_layer = this->getChildByName("PlayLayer");
    auto *score = this->getChildByName<ui::TextAtlas*>("ScoreLabel");
    
    score->setString(std::to_string(_current_score));
    
    auto loading_bar = play_scene_layer->getChildByName<ui::LoadingBar*>("LoadingBar_1");
    loading_bar->setPercent(100.0f * ((double)_current_score / (double)_max_score) + 5.0f);
    
    
    //Millisec単位で計測開始からの時間を取得
    double elapse = StopWatch::getInstance()->currentTime();
    double music_elapse = 1000.0 *AudioManager::getInstance()->getCurrentTime();
    
    CCLOG("StopWatch: %lf, Music: %lf", elapse, music_elapse);
    
    //イテレータで_notes_vectorを最初から最後まで探索
    //そして、今の時間(ms)を超えたノーツが存在する場合、新しいノーツを生成する
    for (std::list<std::queue<ValueMapPtr>>::iterator it = _notes_vector.begin(); it != _notes_vector.end();)
    {
        if(it->front()->at("starttime").asDouble() - _notes_speed_ms - _latency_ms < elapse)
        {
            createNotes(*(it->front()));
            it->pop();
            
            if(it->empty())
            {
                it = _notes_vector.erase(it);
                continue;
            }
        }
        it++;
    }
}


void PlayScene::createNotes(const ValueMap& map)
{
    Vec2 direction = _direction_unit_vector[map.at("lane").asInt()];

    Note *note = Note::create(map, direction);

    //画面の判定外に出た場合の処理
    note->setOutDisplayedCallback(CC_CALLBACK_1(PlayScene::noteOutDisplayedCallback, this));

    //タップ判定を行った後の処理
    note->setTouchCallback(CC_CALLBACK_1(PlayScene::noteTouchCallback, this));


    //ロングノーツを離したときの処理
    note->setReleaseCallback(CC_CALLBACK_1(PlayScene::noteReleaseCallback, this));


    note->setIsFront(_displayed_notes[note->getLane()].empty() ? true : false);
    _displayed_notes[note->getLane()].push(note);

    addChild(note);
}

void PlayScene::applicationDidEnterBackground()
{
    AudioManager::getInstance()->pause(AudioManager::BGM);
    unscheduleUpdate();
    StopWatch::getInstance()->pause();


    this->pause();
    //Director::getInstance()->pause();
    auto video_player =  this->getChildByName<experimental::ui::VideoPlayer*>("VideoLayer");
    video_player->pause();
}


void PlayScene::applicationWillEnterForeground()
{
    scheduleUpdate();
    auto video_player =  this->getChildByName<experimental::ui::VideoPlayer*>("VideoLayer");
    video_player->resume();
    this->resume();


    AudioManager::getInstance()->resume(AudioManager::BGM);
    StopWatch::getInstance()->resume();
}

void PlayScene::finishCallBack(int audioId, std::string fileName)
{
    Director::getInstance()->purgeCachedData();
    cocos2d::experimental::AudioEngine::uncacheAll();
    Scene* home_scene = HomeScene::createScene(ViewScene::Live);
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, home_scene, Color3B::BLACK));
    StopWatch::getInstance()->stop();
}

void PlayScene::noteOutDisplayedCallback(const Note& note)
{
    Sprite *judge_sprite = this->getChildByName<Sprite*>("JudgeSprite");
    if(judge_sprite != nullptr)
        removeChild(judge_sprite);
    auto overSprite = getChildByName<Sprite*>("OverPerfect");
    if(overSprite != nullptr) removeChild(overSprite);
    //Missの処理を行う
    createJudgeSprite(NoteJudge::MISS);
    
    this->_displayed_notes[note.getLane()].pop();
    
    //もし、次のノートが存在するのならば、それが先頭になる
    if(!this->_displayed_notes[note.getLane()].empty())
    {
        Note* next_note = this->_displayed_notes[note.getLane()].front();
        next_note->setIsFront(true);
    }
}

void PlayScene::noteTouchCallback(const Note& note)
{
    Sprite *judge_sprite = this->getChildByName<Sprite*>("JudgeSprite");
    if(judge_sprite != nullptr)
        removeChild(judge_sprite);
    auto over_sprite = getChildByName<Sprite*>("OverPerfect");
    if(over_sprite != nullptr) removeChild(over_sprite);
    
    //判定とタッチのエフェクトを表示する
    createJudgeSprite(note.getResult());

    if(note.isLongNotes()) return;

    //ここからはロングノーツじゃない場合の処理
    createTapFx(note.getChildByName<Sprite*>("BaseNotes")->getPosition());
    this->_displayed_notes[note.getLane()].pop();

    //もし、次のノートが存在するのならば、それが先頭になる
    if(!this->_displayed_notes[note.getLane()].empty())
    {
        Note* next_note = this->_displayed_notes[note.getLane()].front();
        next_note->setIsFront(true);
    }

}

void PlayScene::noteReleaseCallback(const Note& note)
{
    Sprite *judge_sprite = this->getChildByName<Sprite*>("JudgeSprite");
    if(judge_sprite != nullptr)
        removeChild(judge_sprite);
    auto over_sprite = getChildByName<Sprite*>("OverPerfect");
    if(over_sprite != nullptr) removeChild(over_sprite);
    
    //判定とタッチのエフェクトを表示する
    createJudgeSprite(note.getResult());
    createTapFx(note.getChildByName<Sprite*>("BaseNotes")->getPosition());
    this->_displayed_notes[note.getLane()].pop();
    
    //もし、次のノートが存在するのならば、それが先頭になる
    if(!this->_displayed_notes[note.getLane()].empty())
    {
        Note* next_note = this->_displayed_notes[note.getLane()].front();
        next_note->setIsFront(true);
    }
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
    Sprite *judge_sprite = Sprite::createWithSpriteFrameName(spritePath);
    judge_sprite->setName("JudgeSprite");
    auto rect = Director::getInstance()->getVisibleSize();
    judge_sprite->setPosition(rect.width / 2, rect.height / 2);
    judge_sprite->setScale(0.42f);
    judge_sprite->setOpacity(0);
    judge_sprite->setLocalZOrder(1);
    auto *action1 = Spawn::create(ScaleTo::create(0.06f, 2.0f),
                                  FadeTo::create(0.06f, 255), NULL);
    
    auto *action2 = FadeOut::create(0.3f);
    addChild(judge_sprite);
    
    judge_sprite->runAction(Sequence::create(action1,
                                             DelayTime::create(0.1f),
                                             action2,
                                             RemoveSelf::create(), NULL));
    
    //Perfectの場合は既存のPERFECTの画像の上にもう一枚上乗せする
    if(judge==NoteJudge::PERFECT)
    {
        Sprite *over_sprite = Sprite::createWithSpriteFrameName("Image/Judge/judging_19.png");
        over_sprite->setName("OverPerfect");
        over_sprite->setPosition(judge_sprite->getPosition());
        over_sprite->cocos2d::Node::setScale(0.5);
        over_sprite->setLocalZOrder(1);
        over_sprite->setOpacity(0);
        over_sprite->setBlendFunc(BlendFunc::ADDITIVE);
        addChild(over_sprite);
        
        over_sprite->runAction(Sequence::create(FadeIn::create(0.06f),
                                                Spawn::create(FadeOut::create(0.1f),
                                                              ScaleTo::create(0.05f, 2.0f),
                                                              NULL),
                                                RemoveSelf::create(),NULL));
    }
}

void PlayScene::createTapFx(Vec2 position)
{
    auto tap_fx = CSLoader::getInstance()->createNode("res/tapFx.csb");
    tap_fx->setLocalZOrder(0);
    /*座標変換*/
    Vec2 re_position = position;
    re_position.x -= tap_fx->getContentSize().width / 2;
    re_position.y -= tap_fx->getContentSize().height / 2;
    tap_fx->setPosition(re_position);
    this->addChild(tap_fx);
    cocostudio::timeline::ActionTimeline* action = cocostudio::timeline::ActionTimelineCache::getInstance()->createAction("res/tapFx.csb");
    action->setLastFrameCallFunc([tap_fx, this](){
        this->removeChild(tap_fx);
    });
    tap_fx->runAction(action);
    action->gotoFrameAndPlay(0, false);
}
