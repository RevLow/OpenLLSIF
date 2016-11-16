//
//  Note.cpp
//  OpenLLSIF
//
//  Created by RevLow on 2015/08/06.
//
//

#include "Note.h"
#include "triangulate.h"
//#include "SimpleAudioEngine.h"
#include <LLAudioEngine/LLAudioEngine.h>
#include "ui/cocosGui.h"
#include "cocostudio/CocoStudio.h"
#include "NoteMovement.h"

#define MSEC_TO_SEC(duration) ((duration) / 1000.0f)
#define SEC_TO_MSEC(duration) ((duration) * 1000.0f)

#pragma mark - Circle type implements

bool Circle::init(const Point p, float r)
{
    radius = r;
    m_obPosition = p;
    setPosition(p);

    return true;
}

bool Circle::containsPoint(const Vec2 p)
{
    Vec2 v1(m_obPosition.x, m_obPosition.y);
    return v1.distance(p) < radius;
}


bool Circle::intersectRect(const Rect rect)
{
    Vec2 p1(
              clampf(m_obPosition.x , rect.origin.x , rect.origin.x + rect.size.width),
              clampf(m_obPosition.y , rect.origin.y , rect.origin.y + rect.size.height));
    Vec2 p2(m_obPosition.x, m_obPosition.y);
    
    return p2.distance(p1) < radius;
}

bool Circle::intersectCircle(Circle *circle)
{
    Vec2 v1(circle->getPoint().x, circle->getPoint().y);
    Vec2 v2(m_obPosition.x, m_obPosition.y);
    
    return v2.distance(v1) < radius + circle->getRadius();
}

DrawNode* Circle::getDrawNode(Color4F color)
{
    DrawNode *draw = DrawNode::create();
    draw->drawDot(Vec2(0,0), radius, color);
    return draw;
}

#pragma mark - Note type implements

Note::FlickerState::FlickerState()
: isIncrease(false),
time(0),
duration(0.28),
easing(SifUtil::decreaseEasing)
{
}

Note::NoteInfo::NoteInfo()
:lane(0),
startTime(0),
endTime(0),
speed(0),
isStar(false),
isParallel(false),
isLongNote(false),
isHolding(false),
direction(Vec2(0,0))
{
    
}

bool Note::init(const ValueMap& jsonInfo)
{
    SpriteFrameCache::getInstance()->addSpriteFramesWithFile("res/PlayUI.plist");
    
    //jsonの情報からノーツの情報を初期化する
    _noteInfo.isStar = jsonInfo.at("hold").asBool();
    _noteInfo.lane = jsonInfo.at("lane").asInt();
    _noteInfo.isParallel = jsonInfo.at("parallel").asBool();
    _noteInfo.startTime = jsonInfo.at("starttime").asDouble();
    _noteInfo.endTime = jsonInfo.at("endtime").asDouble();
    _noteInfo.isLongNote = jsonInfo.at("longnote").asBool();
    _noteInfo.speed = jsonInfo.at("speed").asDouble();
    _latency = jsonInfo.at("latency").asFloat();

    // 1秒あたりの移動数を計算
    auto theta = SifUtil::BETWEEN_UNITS_ANGLE * _noteInfo.lane;
    _noteInfo.direction = Vec2(-SifUtil::UNIT_RADIUS * cos(MATH_DEG_TO_RAD(theta)) / MSEC_TO_SEC(_noteInfo.speed), -SifUtil::UNIT_RADIUS * sin(MATH_DEG_TO_RAD(theta)) / MSEC_TO_SEC(_noteInfo.speed));
    
    //ノーツ情報の設定
    int type = jsonInfo.at("type").asInt();
    createNotesSprite((NoteType)type);
    
    auto size = Director::getInstance()->getVisibleSize();
    _visibleRect = Rect(0,0,size.width, size.height);
    _judgeResult = NoteJudge::NON;
    _longNotesHoldId = -1;
    _elapsedTime = LLAudioEngine::getInstance()->tellBackgroundMusic();
    
    this->scheduleUpdate();
    
    return true;
}

/**
 *  開始地点とノーツのタイプを指定し、スプライトを作成する
 *  @param initVec [Vec2] 初期地点
 *  @param type    [int]  ノーツのタイプ(0: smile, 1: cool, 2: pure)
 */
void Note::createNotesSprite(const NoteType type)
{
    //ノーツ画像の作成
    Sprite* note;
    switch (type)
    {
        case SMILE:
            note = Sprite::createWithSpriteFrameName("Image/notes/smile_03.png");
            break;
        case COOL:
            note = Sprite::createWithSpriteFrameName("Image/notes/cool_03.png");
            break;
        case PURE:
            note = Sprite::createWithSpriteFrameName("Image/notes/pure_03.png");
            break;
        default:
            break;
    }
    
    //同時ノーツの処理
    if(_noteInfo.isParallel)
    {
        Sprite* parallel = Sprite::createWithSpriteFrameName("Image/notes/longbar_03.png");
        parallel->setPosition(32.0f, 32.0f);
        note->addChild(parallel);
    }
    
    //星付きノーツの処理
    if(_noteInfo.isStar)
    {
        Sprite* star = Sprite::createWithSpriteFrameName("Image/notes/star_03.png");
        star->setPosition(32.0f, 32.0f);
        note->addChild(star);
    }

    note->setName("BaseNotes");
    addChild(note);

    //アニメーションの処理
    note->setVisible(false);
    note->setPosition(SifUtil::initVec);
    note->setScale(0.1f);
    auto scaleToAction = ScaleTo::create(MSEC_TO_SEC(_noteInfo.speed), 2.0f);
    auto sequenceAction = Sequence::create(DelayTime::create(0.03), CallFunc::create([note]()
                                                                                      {
                                                                                          note->setVisible(true);
                                                                                      }), NULL);
    auto moveAction = NoteMovement::create(_noteInfo.direction);
    auto spawnAction = Spawn::create(scaleToAction, sequenceAction, NULL);
    spawnAction->setTag(ActionKey::Simple);
    
    note->runAction(spawnAction);
    Director::getInstance()->getActionManager()->addAction(moveAction, note, false);

    //ロングノーツの終端画像を追加
    if(_noteInfo.isLongNote)
    {
        Sprite* sp = Sprite::createWithSpriteFrameName("Image/notes/longnotes_03.png");
        sp->setName("EndNotes");
        sp->setVisible(false);
        sp->setScale(0.1f);
        sp->setPosition(SifUtil::initVec);
        addChild(sp);
    }
}

/**
 *  ノーツが判定外に出たときに呼ばれるコールバックを設定
 *
 *  @param f コールバック関数
 */
void Note::setOutDisplayedCallback(const std::function<void(const Note&)> &f)
{
    this->_callbackFunc = f;
}

/**
 *  タップを行ったときに呼ばれるコールバックを設定
 *
 *  @param f コールバック関数
 */
void Note::setTouchCallback(const std::function<void (const Note &)> &f)
{
    this->_touchCallbackFunc = f;
}

/**
 *  指を離したときに呼ばれるコールバック関数を設定
 *
 *  @param  f コールバック関数
 */
void Note::setReleaseCallback(const std::function<void (const Note &)> &f)
{
    this->_releaseCallbackFunc = f;
}


/**
 *  シングルタップを検出したときに呼ばれるメソッド
 *
 *  @param touch_id [int] タッチした指のID
 *  @return タッチに成功の場合: true
 */
bool Note::touchBeginAction(int touch_id)
{
    //タッチの判定を行う
    _judgeResult = startJudge();
    if(_judgeResult == NON)
    {
        return false;
    }
    
    //7. タップ判定のコールバック関数実行
    if (_touchCallbackFunc != nullptr)
    {
        _touchCallbackFunc(*this);
    }

    //8. ロングノーツかの判定
    if(_noteInfo.isLongNote)
    {
        _longNotesHoldId = touch_id;
        _noteInfo.isHolding = true;
    }
    else
    {
        this->unscheduleUpdate();
        removeFromParentAndCleanup(true);
    }

    return true;
}

/**
 *  タップ中に指が動いたときに呼ばれる
 *
 *  @param touch_id [int] タッチした指のID
 */
void Note::touchMoveAction(int touch_id)
{
    if (!_noteInfo.isHolding)
    {
        return;
    }
    
    //もともとつかんでいた指のIDと異なる場合
    if(touch_id != _longNotesHoldId)
    {
        return;
    }
    
    //4. タッチの判定を行う
    _judgeResult = endJudge();
    
    //7. タップ判定のコールバック関数実行
    if (_releaseCallbackFunc != nullptr)
    {
        _releaseCallbackFunc(*this);
    }

    this->unscheduleUpdate();
    
    //親から削除
    removeFromParentAndCleanup(true);
}


/**
 *  シングルタップを離したときに呼ばれるメソッド
 *
 *  @param touch 離した指の情報
 *  @param event event description
 */
void Note::touchEndAction(int touch_id)
{
    if (!_noteInfo.isLongNote || !_noteInfo.isHolding)
    {
        return;
    }
    
    //もともとつかんでいた指のIDと異なる場合
    if(touch_id != _longNotesHoldId)
    {
        return;
    }
    
    //4. タッチの判定を行う
    _judgeResult = endJudge();

    //7. タップ判定のコールバック関数実行
    if (_releaseCallbackFunc != nullptr)
    {
        _releaseCallbackFunc(*this);
    }
    auto node = getChildByName("longNotesFx");
    removeChild(node);
    

    this->unscheduleUpdate();
    removeFromParentAndCleanup(true);
}

/**
 *  タップしたときの判定を行うメソッド
 *  判定の時間範囲は以下の目的地からの周囲Nピクセルで決定している
 *  PERFECT 16px
 *  GREAT   40px
 *  GOOD    64px
 *  BAD     112px
 *  @return 判定結果
 */
NoteJudge Note::startJudge()
{
    double elapsed = fabs((_noteInfo.startTime-_latency) - _elapsedTime);
    
    NoteJudge rtn;
    if (elapsed < _noteInfo.speed * 0.04)
    {
        rtn = NoteJudge::PERFECT;
    }
    else if(elapsed >= _noteInfo.speed * 0.04 && elapsed < _noteInfo.speed * 0.10)
    {
        rtn = NoteJudge::GREAT;

    }
    else if(elapsed >= _noteInfo.speed * 0.10 && fabs(elapsed) < _noteInfo.speed * 0.16)
    {
        rtn = NoteJudge::GOOD;
    }
    else if(elapsed >= _noteInfo.speed * 0.16 && elapsed < _noteInfo.speed * 0.28)
    {
        rtn = NoteJudge::BAD;
    }
    else
    {
        rtn = NoteJudge::NON;
    }
    
    return rtn;
}
/**
 *  リリースしたときの判定を行う
 *
 *  @return 判定結果
 */
NoteJudge Note::endJudge()
{
    double elapsed = fabs((_noteInfo.endTime - _latency) - _elapsedTime);
    
    NoteJudge rtn;
    if (elapsed < _noteInfo.speed * 0.04)
    {
        rtn = NoteJudge::PERFECT;
    }
    else if(elapsed >= _noteInfo.speed * 0.04 && elapsed < _noteInfo.speed * 0.10)
    {
        rtn = NoteJudge::GREAT;
        
    }
    else if(elapsed >= _noteInfo.speed * 0.10 && fabs(elapsed) < _noteInfo.speed * 0.16)
    {
        rtn = NoteJudge::GOOD;
    }
    else if(elapsed >= _noteInfo.speed * 0.16)
    {
        rtn = NoteJudge::BAD;
    }
    
    return rtn;
}

#pragma mark Animation Methods

void Note::update(float frame)
{
    double elapsed = SEC_TO_MSEC(frame);
    _elapsedTime += elapsed;

    Sprite* note = this->getChildByName<Sprite*>("BaseNotes");

    if(_noteInfo.isLongNote)
      updateLongNote(elapsed, note);
    else
      updateSimpleNote(note);
}

void Note::updateSimpleNote(Sprite* note)
{
    Vec2 currentPos = note->getPosition();

    //画面判定外処理
    if(!_visibleRect.containsPoint(currentPos))
    {
        if (_callbackFunc != nullptr)
        {
            _callbackFunc(*this);
        }
        
        note->stopActionByTag(ActionKey::Simple);
        this->unscheduleUpdate();
        this->removeFromParentAndCleanup(true);
    }
}

void Note::updateLongNote(const double& elapsed, Sprite* note)
{
    Vec2 currentPos = note->getPosition();
    Sprite* sp = this->getChildByName<Sprite*>("EndNotes");
    
    //終端画像の処理
    if (_elapsedTime + _noteInfo.speed + _latency > _noteInfo.endTime)
    {
        auto runningAction = sp->getActionByTag(ActionKey::LongNotes);
        /*
         * この処理は一度しか呼ばないようにアクションにタグを設定し、そのタグのアクションが実行済みかで
         * 分岐判定を行う
         */
        if (!runningAction)
        {
            sp->setVisible(false);
            auto scaleToAction = ScaleTo::create(MSEC_TO_SEC(_noteInfo.speed), 2.0f);
            auto sequenceAction = Sequence::create(DelayTime::create(0.03), CallFunc::create([sp]()
                                                                                             {
                                                                                                 sp->setVisible(true);
                                                                                             }), NULL);
            auto moveAction = NoteMovement::create(_noteInfo.direction);
            auto spawnAction = Spawn::create(scaleToAction, sequenceAction, NULL);
            spawnAction->setTag(ActionKey::LongNotes);
            sp->runAction(spawnAction);
            Director::getInstance()->getActionManager()->addAction(moveAction, sp, false);
        }
    }

    renderFilledPolygon(note, sp);
    
    //つかんでいる状態の場合は点滅を行う
    if(_noteInfo.isHolding)
    {
        FilledPolygon* poly = getChildByName<FilledPolygon*>("LongnotesLine");
        poly->setBlendFunc((BlendFunc){GL_SRC_ALPHA, GL_ONE});
        poly->setTexture(Director::getInstance()->getTextureCache()->addImage("Image/longNoteLine_Brightness.png"));
        flickerPolygon(poly, elapsed);
        
        Vec2 v1 = currentPos - SifUtil::initVec;
        Vec2 v2(0, -1);
        float angle = MATH_RAD_TO_DEG(v1.getAngle(v2));
        
        auto particle = getChildByName<ParticleSystemQuad*>("longNotesParticle");
        if(particle == nullptr)
        {
            this->runAction(Sequence::create(DelayTime::create(0.05),
                                             CallFunc::create([currentPos, angle, this](){
                                                                    auto particle = ParticleSystemQuad::create("particle_texture.plist");
                                                                    particle->setBlendFunc((BlendFunc){GL_ONE, GL_ONE});
                                                                     
                                                                    particle->setAutoRemoveOnFinish(true);
                                                                    particle->setPosition(currentPos);
                                                                    particle->setRotation(angle);
                                                                    particle->setScale(0.9);
                                                                    particle->setName("longNotesParticle");
                                                                    this->addChild(particle);
                                                                }), NULL));
        }
        
        auto lnFx = getChildByName<Layer*>("longNotesFx");
        if (lnFx == nullptr)
        {
            auto lnFx = CSLoader::getInstance()->createNode("res/ln_hold.csb");
            cocostudio::timeline::ActionTimeline* action = cocostudio::timeline::ActionTimelineCache::getInstance()->createAction("res/ln_hold.csb");
            lnFx->setAnchorPoint(Vec2(0.5, 0.5));
            lnFx->setPosition(currentPos);
            lnFx->setRotation(angle);
            lnFx->setName("longNotesFx");
            this->addChild(lnFx);
            
            lnFx->runAction(action);
            action->gotoFrameAndPlay(0, true);
        }
        
        
        if((currentPos - SifUtil::initVec).length() >= (SifUtil::unitPosition(_noteInfo.lane) - SifUtil::initVec).length())
        {
            note->stopActionByTag(ActionKey::Simple);
            note->setPosition(SifUtil::unitPosition(_noteInfo.lane));
        }
    }
    else
    {
        //画面外判定
        updateSimpleNote(note);
    }
}

void Note::renderFilledPolygon(Sprite* startNoteSprite, Sprite* endNoteSprite)
{
    Vec2 currentPos = startNoteSprite->getPosition();
    Vec2 endPosition = endNoteSprite->getPosition();
    float angle = MATH_DEG_TO_RAD( 90.0 - (SifUtil::BETWEEN_UNITS_ANGLE * _noteInfo.lane) );
    
    std::vector<Vec2> vList = std::vector<Vec2>();
    std::vector<Vec2> uvCoordinate = std::vector<Vec2>();
    
    float scale1 = startNoteSprite->getScale() / 2.0;
    float scale2 = endNoteSprite->getScale() / 2.0;
    vList.resize(4);
    uvCoordinate.resize(4);
    
    vList[0].x = floor((currentPos.x + (scale1 * 60.0f) * cos(angle)) + 0.5);
    vList[0].y = floor((currentPos.y - (scale1 * 60.0f) * sin(angle)) + 0.5);

    vList[1].x = floor((currentPos.x - (scale1 * 60.0f) * cos(angle)) + 0.5);
    vList[1].y = floor((currentPos.y + (scale1 * 60.0f) * sin(angle)) + 0.5);
    
    vList[2].x = floor((endPosition.x - (scale2 * 60.0f) * cos(angle)) + 0.5);
    vList[2].y = floor((endPosition.y + (scale2 * 60.0f) * sin(angle)) + 0.5);
    
    vList[3].x = floor((endPosition.x + (scale2 * 60.0f) * cos(angle)) + 0.5);
    vList[3].y = floor((endPosition.y - (scale2 * 60.0f) * sin(angle)) + 0.5);


    uvCoordinate[0] = Vec2(1, 1);
    uvCoordinate[1] = Vec2(0, 1);
    uvCoordinate[2] = Vec2(0, 0);
    uvCoordinate[3] = Vec2(1, 0);
    
    FilledPolygon *poly = getChildByName<FilledPolygon*>("LongnotesLine");//加算合成するポリゴン
    
    //すでにポリゴンが描画されている場合は頂点座標とUV座標を変更するだけにする
    if (poly == nullptr )
    {
        Texture2D *texture = Director::getInstance()->getTextureCache()->addImage("Image/longNoteLine_07.png");
        poly = FilledPolygon::create(texture, vList, uvCoordinate);
        poly->setOpacity(127);
        poly->setName("LongnotesLine");
        addChild(poly);
    }
    else
    {
        poly->setTexturePolygon(vList, uvCoordinate);
    }
}

void Note::flickerPolygon(FilledPolygon* poly, double sleepTime)
{
    _lnFlash.time += MSEC_TO_SEC(sleepTime);
    if(_lnFlash.time >= _lnFlash.duration)
    {
        // 増加と減少を交換する
        _lnFlash.isIncrease = !_lnFlash.isIncrease;
        _lnFlash.time = 0;
        constexpr float t1 = 28.0 / 60.0;
        constexpr float t2 = 17.0 / 60.0;
        _lnFlash.duration = _lnFlash.isIncrease ?  t1 : t2;
        _lnFlash.easing = _lnFlash.isIncrease ? SifUtil::increaseEasing : SifUtil::decreaseEasing;
    }
    
    float t = _lnFlash.time / _lnFlash.duration;
    float opacity = _lnFlash.easing(t);
    if(opacity <= 0.0) opacity = 0.0;
    if(opacity >= 1.0) opacity = 1.0;

    poly->setOpacity(127 * opacity);
}