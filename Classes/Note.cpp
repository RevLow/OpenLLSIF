//
//  Note.cpp
//  OpenLLSIF
//
//  Created by RevLow on 2015/08/06.
//
//

#include "Note.h"
#include "AudioManager.h"
#include "PRFilledPolygon.h"
#include "triangulate.h"

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

/* ===============================ここからNote型のメソッド====================*/

bool Note::init(ValueMap jsonInfo, cocos2d::Vec2 unitVec)
{
    SpriteFrameCache::getInstance()->addSpriteFramesWithFile("res/PlayUI.plist");
    
    //jsonの情報からノーツの情報を初期化する
    _isStar = jsonInfo.at("hold").asBool();
    _lane = jsonInfo.at("lane").asInt();
    _isParallel = jsonInfo.at("parallel").asBool();
    _startTime = jsonInfo.at("starttime").asDouble();
    _endTime = jsonInfo.at("endtime").asDouble();
    _isLongnote = jsonInfo.at("longnote").asBool();
    _speed = jsonInfo.at("speed").asDouble();
    latency = jsonInfo.at("latency").asFloat();
    _destination = Vec2(jsonInfo.at("destinationX").asFloat(),jsonInfo.at("destinationY").asFloat());
    int type = jsonInfo.at("type").asInt();
    
    limitArea = Director::getInstance()->getWinSizeInPixels();
    
    Vec2 norm = unitVec;
    Vec2 initVec(480, 480);

    // 1msごとの進行速度を減らし、
    // 開始地点を1ms分目的地方向に進めておく
    // 最初にでてきた瞬間を中心から離すことが出来る
    norm.normalize();
    initVec += norm;
    unitVec -= norm;
    _unitVec = unitVec / _speed;
    
    createNotesSprite(initVec, type);
    
    //タッチイベントリスナーを作成
    auto listener = cocos2d::EventListenerTouchOneByOne::create();
    listener->setEnabled(true);
    listener->onTouchBegan = CC_CALLBACK_2(Note::onTouchBegan, this);
    if(_isLongnote)
    {
        listener->onTouchMoved = CC_CALLBACK_2(Note::onTouchMoved, this);
        listener->onTouchCancelled = CC_CALLBACK_2(Note::onTouchEnded, this);
        listener->onTouchEnded = CC_CALLBACK_2(Note::onTouchEnded, this);
    }
    this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, this);
    
    //時間計測開始
    startTimeCount = endTimeCount = StopWatch::getInstance()->currentTime();
    this->scheduleUpdate();
    
    result = NoteJudge::NON;
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
    if(_isParallel)
    {
        Sprite* parallel = Sprite::createWithSpriteFrameName("Image/notes/longbar_03.png");
        parallel->setPosition(35.0f, 35.0f);
        note->addChild(parallel);
    }
    
    //星付きノーツの処理
    if(_isStar)
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
    auto scale_to_action = ScaleTo::create(_speed / 1000.0f, 2.0f);
    auto sequence_action = Sequence::create(DelayTime::create(0.03), CallFunc::create([note]()
                                                                                      {
                                                                                          note->setVisible(true);
                                                                                      }), NULL);
    
    note->runAction(Spawn::create(scale_to_action,sequence_action, NULL));
    
    //ロングノーツの終端画像を追加
    if(_isLongnote)
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
 *  @param touch タッチした指の情報
 *  @param event event description
 *
 *  @return タッチに成功の場合: true
 */
bool Note::onTouchBegan(Touch *touch, Event *event)
{
    
    //もしも先頭要素でない場合は何もしない
    if (!isFrontOfLane)
    {
        return false;
    }


    //位置を取得する(タッチ座標は絶対座標なためこのレイヤーの相対座標に変換して扱う)
    Vec2 pos = touch->getLocation();
    
    //タップした点がアイコン内に入っていない場合はこれより下の処理を行わない
    if(!isPointContain(pos))
    {
        return false;
    }
    
    //タッチの判定を行う
    NoteJudge j = startJudge();
    if(j == NON)
    {
        return false;
    }

    //5. タッチの判定により処理を変える
    std::string fullpath;
    switch (j)
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
            break;
    }

    //6. 音の再生
    AudioManager::getInstance()->play(fullpath, AudioManager::SE);

    //7. タップ判定のコールバック関数実行
    if (_touchCallbackFunc != nullptr)
    {
        _touchCallbackFunc(*this);
    }

    //8. ロングノーツかの判定
    if(_isLongnote)
    {
        _longNotesHoldId = touch->getID();
        _longnotesHold = true;
    }
    else
    {
        this->unscheduleUpdate();
        //そうじゃない場合は親から削除
        removeFromParentAndCleanup(true);
    }

    return true;
}

/**
 *  タップ中に指が動いたときに呼ばれる
 *
 *  @param touch <#touch description#>
 *  @param event <#event description#>
 */
void Note::onTouchMoved(Touch *touch, Event *event)
{
    if ( !_longnotesHold)
    {
        return;
    }
    
    //もともとつかんでいた指のIDと異なる場合
    if(touch->getID() != _longNotesHoldId)
    {
        return;
    }
    
    //2. 位置を取得する(タッチ座標は絶対座標なためこのレイヤーの相対座標に変換して扱う)
    Vec2 pos = touch->getLocation();
    
    //タップした点と基準ベクトルの角度が、タップ可能の角度内に入っている場合は何も行わない
    if(isPointContain(pos))
    {
        return;
    }
    
    //4. タッチの判定を行う
    NoteJudge j = endJudge();
    //5. タッチの判定により処理を変える
    std::string fullpath;
    switch (j)
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
        case NoteJudge::NON:
        case NoteJudge::BAD:
            fullpath=FileUtils::getInstance()->fullPathForFilename("Sound/SE/bad.mp3");
            break;
        default:
            break;
    }
    
    //6. 音の再生
    AudioManager::getInstance()->play(fullpath, AudioManager::SE);
    
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
void Note::onTouchEnded(Touch *touch, Event *event)
{
    if (!_isLongnote || !_longnotesHold)
    {
        return;
    }
    

    //もともとつかんでいた指のIDと異なる場合
    if(touch->getID() != _longNotesHoldId)
    {
        return;
    }
    
    //4. タッチの判定を行う
    NoteJudge j = endJudge();

    //5. タッチの判定により処理を変える
    std::string fullpath;
    switch (j)
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
        case NoteJudge::NON:
        case NoteJudge::BAD:
            fullpath=FileUtils::getInstance()->fullPathForFilename("Sound/SE/bad.mp3");
            break;
        default:
            break;
    }

    //6. 音の再生
    AudioManager::getInstance()->play(fullpath, AudioManager::SE);

    //7. タップ判定のコールバック関数実行
    if (_releaseCallbackFunc != nullptr)
    {
        _releaseCallbackFunc(*this);
    }
    
    this->unscheduleUpdate();
    
    
    //そうじゃない場合は親から削除
    removeFromParentAndCleanup(true);

}

/**
 *  ある点がタップ可能な範囲内に入っているかを判定する
 *  判定方法はある点を基準にした半径r内に目的のSpriteが入っているかで判定する
 *  Spriteは128pxなので半径R = (64+offset値)で決定している
 *
 *  @param pos タップした点
 *
 *  @return true: 半径が交差している, false: 判定ミス
 */
bool Note::isPointContain(Vec2 pos)
{
    Circle *finger_circle = Circle::create(this->convertToWorldSpace(pos), 38); //半径r = 15pxで仮決定
    
    //TODO: 親からスプライトを取得しているので、ほかのやり方で位置を取得するべき
    Vec2 baseVec = getParent()->getChildByName("PlayLayer")->getChildByName<Sprite*>(std::to_string(_lane+1))->getPosition();
    Circle *target_cirlce = Circle::create(this->convertToWorldSpace(baseVec), 64); //offset4pxで仮決定
    
    return target_cirlce->intersectCircle(finger_circle);
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
    double now = StopWatch::getInstance()->currentTime();

    double elapsed = fabs((_startTime-latency) - now);
    
    //もしも判定外の場合はNONを返す
    
    if (elapsed >= _speed * 0.28) {
        
        return NoteJudge::NON;
    }
    
    NoteJudge rtn;
    if (elapsed < _speed*0.04)
    {
        rtn = NoteJudge::PERFECT;
    }
    else if(elapsed >=_speed*0.04 && elapsed < _speed*0.10)
    {
        rtn = NoteJudge::GREAT;

    }
    else if(elapsed >= _speed*0.10 && fabs(elapsed) < _speed*0.16)
    {
        rtn = NoteJudge::GOOD;
    }
    else if(elapsed >= _speed*0.16 && elapsed < _speed*0.28)
    {
        rtn = NoteJudge::BAD;
    }
    
    result = rtn;
    
    return rtn;
}
/**
 *  リリースしたときの判定を行う
 *
 *  @return 判定結果
 */
NoteJudge Note::endJudge()
{
    double now = StopWatch::getInstance()->currentTime();

    double elapsed = fabs((_endTime-latency) - now);
    
    NoteJudge rtn;
    if (elapsed < _speed*0.04)
    {
        rtn = NoteJudge::PERFECT;
    }
    else if(elapsed >=_speed*0.04 && elapsed < _speed*0.10)
    {
        rtn = NoteJudge::GREAT;
        
    }
    else if(elapsed >= _speed*0.10 && fabs(elapsed) < _speed*0.16)
    {
        rtn = NoteJudge::GOOD;
    }
    else if(elapsed >= _speed*0.16)
    {
        rtn = NoteJudge::BAD;
    }
    

    //Director::getInstance()->purgeCachedData();//無駄なテクスチャキャッシュは消去してメモリを押さえる
    //もしコールバックが存在するなら
    //if(_callbackFunc) _callbackFunc();
    
    result = rtn;
    
    return rtn;
}


void Note::update(float frame)
{

    double now = StopWatch::getInstance()->currentTime();
    double elapsed = now - startTimeCount;
    //RenderTexture* note = this->getChildByName<RenderTexture*>("BaseNotes");
    Sprite* note = this->getChildByName<Sprite*>("BaseNotes");
    cocos2d::Vec2 currentPos = note->getPosition();

    if(_isLongnote)
    {
        Sprite* sp = this->getChildByName<Sprite*>("EndNotes");
        if (now + _speed + latency > _endTime)
        {
            _endOfPoint += _unitVec*(now - endTimeCount);
            sp->setPosition(_endOfPoint);
            
            auto runningAction = sp->getActionByTag(1);
            
            if(!runningAction && now < _endTime)
            {
                sp->setVisible(false);
                auto action = Spawn::create(cocos2d::ScaleTo::create(_speed / 1000.0f, 2.0f),
                                            Sequence::create(DelayTime::create(0.02), CallFunc::create([sp](){sp->setVisible(true);}), NULL)
                                            , NULL);
                action->setTag(1);
                sp->runAction(action);
                
            }
        }
        
        Vec2 v1 = currentPos - Vec2(480,480);
        Vec2 v2(0, -1);
        
        double length = (currentPos - _endOfPoint).length();
        float angle = MATH_RAD_TO_DEG(v1.getAngle(v2));


        length += 10;
        std::vector<Vec2> vList = std::vector<Vec2>();
        std::vector<Vec2> uvCoordinate = std::vector<Vec2>();
        Vec2 v = Vec2(0, -1*length);
   
        vList.push_back(Vec2(-30.0f * note->getScale(), v.y));
        vList.push_back(Vec2(30.0f * note->getScale(), v.y));
        vList.push_back(Vec2(30.0f * sp->getScale(),0));
        vList.push_back(Vec2(-30.0f * sp->getScale() ,0));
        
        uvCoordinate.push_back(Vec2(0,0));
        uvCoordinate.push_back(Vec2(1,0));
        uvCoordinate.push_back(Vec2(1,1));
        uvCoordinate.push_back(Vec2(0,1));

        
        
        FilledPolygon *poly = getChildByName<FilledPolygon*>("LongnotesLine");//加算合成するポリゴン
        
        
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
        
        //つかんでいる状態の場合は点滅を行う
        if(_longnotesHold)
        {
            poly->setBlendFunc((BlendFunc){GL_SRC_ALPHA, GL_ONE});
            poly->setTexture(Director::getInstance()->getTextureCache()->addImage("Image/longNoteLine_Brightness.png"));
//
            //単純なsin関数での透明度
            static float x = MATH_DEG_TO_RAD(0);
            static float time = 0.0f;
            
            float middle = 0.5f;
            float opacity = sin(x) * middle + middle; // 0~255
            opacity *= 0.5;
            if(x >= FLT_MAX) x=0;
            //opacity += 0.2;
            GLubyte opacityByte = static_cast<GLubyte>(opacity * 255.0f);
            
            poly->setOpacity(opacityByte);
            if(opacity >= 1.0f && time < 20.0f)
            {
                time += elapsed;
            }
            else
            {
                x += MATH_DEG_TO_RAD(8.0);                 //数値はよしなに
                time = 0.0f;
            }
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
            }*/
        }



        
        endTimeCount = now;
    }
    
    if(!_longnotesHold)
    {
        double time = _startTime - now;
        //if(currentPos.x < 0 || currentPos.x > limitArea.width || currentPos.y < 0 || currentPos.y > limitArea.height)
        if(time < -_speed * 0.30)
        {
            if (_callbackFunc != nullptr)
            {
                _callbackFunc(*this);
            }
            
            //もし画面判定外にでたら
            this->unscheduleUpdate();
            this->removeFromParentAndCleanup(true);
        }
        else
        {
            currentPos += _unitVec*elapsed;
            note->setPosition(currentPos);
        }
    }
    else
    {
        //もし押しても目的地に着いてない場合はそのまま進める
        if(now < _startTime)
        {
            currentPos += _unitVec*elapsed;
            if(currentPos.getDistance(Vec2(480,480)) > _destination.getDistance(Vec2(480,480))) currentPos = _destination;
            note->setPosition(currentPos);
        }

    }
    
    startTimeCount = now;
}