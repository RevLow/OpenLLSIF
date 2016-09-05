//
//  Note.cpp
//  OpenLLSIF
//
//  Created by RevLow on 2015/08/06.
//
//

#include "Note.h"
//#include "AudioManager.h"
#include "triangulate.h"
#include "SimpleAudioEngine.h"

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
: theta(0),
time(0)
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

bool Note::init(const ValueMap& jsonInfo, cocos2d::Vec2 unitVec)
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
    latency = jsonInfo.at("latency").asFloat();
    _destination = Vec2(jsonInfo.at("destinationX").asFloat(),jsonInfo.at("destinationY").asFloat());
    int type = jsonInfo.at("type").asInt();

    
    Vec2 norm = unitVec;
    Vec2 initVec(480, 480);

    // 1msごとの進行速度を減らし、
    // 開始地点を1ms分目的地方向に進めておく
    // 最初にでてきた瞬間を中心から離すことが出来る
    norm.normalize();
    initVec += norm;
    unitVec -= norm;
    _noteInfo.direction = unitVec / _noteInfo.speed;
    
    createNotesSprite(initVec, type);
    
    //時間計測開始
    startTimeCount = endTimeCount = CocosDenshion::SimpleAudioEngine::getInstance()->getCurrentTime() * 1000.0;//StopWatch::getInstance()->currentTime();
    this->scheduleUpdate();
    
    _judgeResult = NoteJudge::NON;
    _longNotesHoldId = -1;
    return true;
}

/**
 *  開始地点とノーツのタイプを指定し、スプライトを作成する
 *  @param initVec [Vec2] 初期地点
 *  @param type    [int]  ノーツのタイプ(0: smile, 1: cool, 2: pure)
 */
void Note::createNotesSprite(Vec2 &initVec, int type)
{
    //ノーツ画像の作成
    Sprite* note;
    switch (type)
    {
        case 0:
            note = Sprite::createWithSpriteFrameName("Image/notes/smile_03.png");
            break;
        case 1:
            note = Sprite::createWithSpriteFrameName("Image/notes/cool_03.png");
            break;
        case 2:
            note = Sprite::createWithSpriteFrameName("Image/notes/pure_03.png");
            break;
        default:
            break;
    }
    
    //同時ノーツの処理
    if(_noteInfo.isParallel)
    {
        Sprite* parallel = Sprite::createWithSpriteFrameName("Image/notes/longbar_03.png");
        parallel->setPosition(35.0f, 35.0f);
        note->addChild(parallel);
    }
    
    //星付きノーツの処理
    if(_noteInfo.isStar)
    {
        Sprite* star = Sprite::createWithSpriteFrameName("Image/notes/star_03.png");
        star->setPosition(35.0f, 35.0f);
        note->addChild(star);
    }

    note->setName("BaseNotes");
    addChild(note);

    //アニメーションの処理
    note->setVisible(false);
    note->setPosition(initVec);
    note->setScale(0.1f);
    auto scaleToAction = ScaleTo::create(_noteInfo.speed / 1000.0f, 2.0f);
    auto sequenceAction = Sequence::create(DelayTime::create(0.03), CallFunc::create([note]()
                                                                                      {
                                                                                          note->setVisible(true);
                                                                                      }), NULL);
    
    note->runAction(Spawn::create(scaleToAction,sequenceAction, NULL));
    
    //ロングノーツの終端画像を追加
    if(_noteInfo.isLongNote)
    {
        _endOfPoint = Vec2(initVec);
        Sprite* sp = Sprite::createWithSpriteFrameName("Image/notes/longnotes_03.png");
        sp->setName("EndNotes");
        sp->setVisible(false);
        sp->setPosition(_endOfPoint);
        sp->setScale(0.1f);
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
    double now = CocosDenshion::SimpleAudioEngine::getInstance()->getCurrentTime() * 1000.0;//StopWatch::getInstance()->currentTime();

    double elapsed = fabs((_noteInfo.startTime-latency) - now);
    
    //もしも判定外の場合はNONを返す
    
    if (elapsed >= _noteInfo.speed * 0.28) {
        
        return NoteJudge::NON;
    }
    
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
    
    return rtn;
}
/**
 *  リリースしたときの判定を行う
 *
 *  @return 判定結果
 */
NoteJudge Note::endJudge()
{
    double now = CocosDenshion::SimpleAudioEngine::getInstance()->getCurrentTime() * 1000.0;//StopWatch::getInstance()->currentTime();

    double elapsed = fabs((_noteInfo.endTime - latency) - now);
    
    NoteJudge rtn;
    if (elapsed < _noteInfo.speed * 0.04)
    {
        rtn = NoteJudge::PERFECT;
    }
    else if(elapsed >=_noteInfo.speed * 0.04 && elapsed < _noteInfo.speed * 0.10)
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

/**
 *  @param frame <#frame description#>
 */
void Note::update(float frame)
{
    double now = CocosDenshion::SimpleAudioEngine::getInstance()->getCurrentTime() * 1000.0;//StopWatch::getInstance()->currentTime();
    double elapsed = now - startTimeCount;
    Sprite* note = this->getChildByName<Sprite*>("BaseNotes");

    if(_noteInfo.isLongNote)
      updateLongNote(now, elapsed, note);
    else
      updateSimpleNote(now, elapsed, note);
    
    startTimeCount = now;
}

void Note::updateNotePosition(Sprite* note, float elapsedTime)
{
    Vec2 currentPos = note->getPosition();
    currentPos += _noteInfo.direction * elapsedTime;
    if(currentPos.getDistance(Vec2(480,480)) > _destination.getDistance(Vec2(480,480)) && _noteInfo.isLongNote) currentPos = _destination;
    note->setPosition(currentPos);
}

void Note::updateSimpleNote(double now, double elapsed, Sprite* note)
{
    Vec2 currentPos = note->getPosition();
    double time = _noteInfo.startTime - now;

    //画面判定外処理
    if(time < -_noteInfo.speed * 0.25)
    {
        if (_callbackFunc != nullptr)
        {
            _callbackFunc(*this);
        }
        
        this->unscheduleUpdate();
        this->removeFromParentAndCleanup(true);
    }
    else
    {
        updateNotePosition(note, elapsed);
    }
}

void Note::updateLongNote(double now, double elapsed, Sprite* note)
{
    Vec2 currentPos = note->getPosition();
    Sprite* sp = this->getChildByName<Sprite*>("EndNotes");
    
    //終端画像の処理
    if (now + _noteInfo.speed + latency > _noteInfo.endTime)
    {
        _endOfPoint += _noteInfo.direction * (now - endTimeCount);
        sp->setPosition(_endOfPoint);
        
        auto runningAction = sp->getActionByTag(1);
        
        /*
         * 終端時間より速度分前の時間で終端画像のスケールアクションを実行する
         * この処理は一度しか呼ばないようにアクションにタグを設定し、そのタグのアクションが実行済みかで
         * 分岐判定を行う
         */
        if(!runningAction && now < _noteInfo.endTime)
        {
            sp->setVisible(false);
            float scaleTimeSec = _noteInfo.speed / 1000.0f;
            auto scaleAction = ScaleTo::create(scaleTimeSec, 2.0f);
            
            //seqActionは最初の数ミリ秒は中心から出てくるため、そこを隠すため表示しないようにしている
            auto seqAction = Sequence::create(DelayTime::create(0.02),
                                              CallFunc::create([sp](){sp->setVisible(true);}), NULL);
            auto action = Spawn::create(scaleAction, seqAction, NULL);
            action->setTag(1);
            sp->runAction(action);
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
        /*
         auto particle = getChildByName<ParticleSystemQuad*>("longNotesParticle");
         if(particle == nullptr)
         {
         particle = ParticleSystemQuad::create("particle_texture.plist");
         //particle->setTexture(Director::getInstance()->getTextureCache()->addImage("longNotesSpark.png"));
         particle->setAutoRemoveOnFinish(true);
         particle->setPosition(currentPos);
         particle->setRotation(angle);
         particle->setSpeed(220);
         particle->setScale(0.7);
         particle->setName("longNotesParticle");
         this->addChild(particle);
         }
        */
        
        //押しても目的地に着いてない場合はそのまま進める
        if(now < _noteInfo.startTime)
        {
            updateNotePosition(note, elapsed);
        }
    }
    else
    {
        //画面外判定
        double time = _noteInfo.startTime - now;
        if(time < -_noteInfo.speed * 0.30)
        {
            if (_callbackFunc != nullptr)
            {
                _callbackFunc(*this);
            }
            
            this->unscheduleUpdate();
            this->removeFromParentAndCleanup(true);
        }
        else
        {
            updateNotePosition(note, elapsed);
        }
    }
    endTimeCount = now;
}

void Note::renderFilledPolygon(Sprite* startNoteSprite, Sprite* endNoteSprite)
{
    Vec2 currentPos = startNoteSprite->getPosition();
    
    Vec2 v1 = currentPos - Vec2(480,480);
    Vec2 v2(0, -1);
    
    double length = (currentPos - _endOfPoint).length();
    float angle = MATH_RAD_TO_DEG(v1.getAngle(v2));
    
    
    length += 10;
    std::vector<Vec2> vList = std::vector<Vec2>();
    std::vector<Vec2> uvCoordinate = std::vector<Vec2>();
    Vec2 v = Vec2(0, -1*length);
    
    vList.push_back(Vec2(-30.0f * startNoteSprite->getScale(), v.y));
    vList.push_back(Vec2(30.0f * startNoteSprite->getScale(), v.y));
    vList.push_back(Vec2(30.0f * endNoteSprite->getScale(),0));
    vList.push_back(Vec2(-30.0f * endNoteSprite->getScale() ,0));
    
    uvCoordinate.push_back(Vec2(0,0));
    uvCoordinate.push_back(Vec2(1,0));
    uvCoordinate.push_back(Vec2(1,1));
    uvCoordinate.push_back(Vec2(0,1));
    
    FilledPolygon *poly = getChildByName<FilledPolygon*>("LongnotesLine");//加算合成するポリゴン
    
    //すでにポリゴンが描画されている場合は頂点座標とUV座標を変更するだけにする
    if (poly == nullptr )
    {
        Texture2D *texture = Director::getInstance()->getTextureCache()->addImage("Image/longNoteLine_07.png");
        poly = FilledPolygon::create(texture, vList, uvCoordinate);
        poly->setName("LongnotesLine");
        addChild(poly);
    }
    else
    {
        poly->setTexturePolygon(vList, uvCoordinate);
    }
    
    poly->setRotation(angle);
    poly->setPosition(_endOfPoint);
}

void Note::flickerPolygon(FilledPolygon* poly, double sleepTime)
{
    //単純なsin関数での透明度
    float middle = 0.5f;
    float opacity = sin(_lnFlash.theta) * middle + middle; // 0~255
    opacity *= 0.5;
    if(_lnFlash.theta >= FLT_MAX) _lnFlash.theta=0;
    GLubyte opacityByte = static_cast<GLubyte>(opacity * 255.0f);
    
    poly->setOpacity(opacityByte);
    if(opacity >= 1.0f && _lnFlash.time < 20.0f)
    {
        _lnFlash.time += sleepTime;
    }
    else
    {
        _lnFlash.theta += MATH_DEG_TO_RAD(8.0);
        _lnFlash.time = 0.0f;
    }
}