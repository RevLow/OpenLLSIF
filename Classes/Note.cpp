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

bool Note::init(ValueMap jsonInfo, cocos2d::Vec2 unitVec)
{
    SpriteFrameCache::getInstance()->addSpriteFramesWithFile("res/PlayUI.plist");

    limitArea = Director::getInstance()->getWinSizeInPixels();

    //ノーツの基本画像からサイズを指定して初期化
    RenderTexture* note = RenderTexture::create(70.0f, 70.0f, Texture2D::PixelFormat::RGBA8888);

    
    //jsonの情報からノーツの情報を初期化する
    _isStar = jsonInfo.at("hold").asBool();
    _lane = jsonInfo.at("lane").asInt();
    _isParallel = jsonInfo.at("parallel").asBool();
    _startTime = jsonInfo.at("starttime").asDouble();
    _endTime = jsonInfo.at("endtime").asDouble();
    _isLongnote = jsonInfo.at("longnote").asBool();
    _speed = jsonInfo.at("speed").asDouble();
    
    int type = jsonInfo.at("type").asInt();
    
    Vec2 norm = unitVec;
    //Vec2 initVec(479.75, 476.34);
    Vec2 initVec(480, 480);

    norm.normalize();
    //norm *= 10     ;
    initVec += norm;
    unitVec -= norm;
    
    
    _unitVec = unitVec / _speed;
    
    //ノーツ画像の作成
    note->begin();
    Sprite* baseNote;
    switch (type)
    {
        case 0:
            baseNote = Sprite::createWithSpriteFrameName("Image/notes/smile_03.png");
            break;
        case 1:
            baseNote = Sprite::createWithSpriteFrameName("Image/notes/cool_03.png");
            break;
        case 2:
            baseNote = Sprite::createWithSpriteFrameName("Image/notes/pure_03.png");
            break;
        default:
            break;
    }
    

     baseNote->retain();
     baseNote->setPosition(35.0, 35.0f);
     //ノーツ画像を書き込み
     baseNote->visit();
    
     if(_isParallel)
     {
         Sprite* parallel = Sprite::createWithSpriteFrameName("Image/notes/longbar_03.png");
         parallel->retain();
         parallel->setPosition(35.0f, 35.0f);
         //スター画像を書き込み
         parallel->visit();
         CC_SAFE_RELEASE(parallel);
     }
    
     if(_isStar)
     {
         Sprite* star = Sprite::createWithSpriteFrameName("Image/notes/star_03.png");
         star->retain();
         star->setPosition(35.0f, 35.0f);
         //スター画像を書き込み
         star->visit();
         CC_SAFE_RELEASE(star);
     }
    note->end();
    CC_SAFE_RELEASE(baseNote);

    
    note->setName("BaseNotes");
    addChild(note);
    note->setVisible(false);
    note->setPosition(initVec);
    note->setScale(0.1f);
    note->runAction(Spawn::create(cocos2d::ScaleTo::create(_speed / 1000.0f, 2.0f),
                                  Sequence::create(DelayTime::create(0.03), CallFunc::create([note](){note->setVisible(true);}), NULL)
                                  , NULL)
                    );
    if(_isLongnote)
    {
        _endOfPoint = Vec2(initVec);
        //キャッシュにテクスチャを登録
        Sprite* sp = Sprite::createWithSpriteFrameName("Image/notes/longnotes_03.png");
        sp->setName("EndNotes");
        sp->setVisible(false);
        sp->setPosition(_endOfPoint);
        sp->setScale(0.1f);
        addChild(sp);
        
      
        //_scaleTick = (2.0-0.1) / _speed;
    }
    
    //時間計測開始
    startTimeCount = endTimeCount = StopWatch::getInstance()->currentTime();
    this->scheduleUpdate();
    
    
    return true;
}

//ノーツが消えるときに呼ばれるコールバック
void Note::setOutDisplayedCallback(const std::function<void()> &f)
{
    this->_callbackFunc = f;
}


/*
 タップしたときの判定を行う関数
 */
NoteJudge Note::StartJudge()
{
    double now = StopWatch::getInstance()->currentTime();
    double offset = 5;
    double elapsed = fabs((_startTime-offset) - now);
    
    //もしも判定外の場合はNONを返す
    
    if (elapsed >= _speed * 0.50) {
        
        return NoteJudge::NON;
    }
    
    NoteJudge rtn;
    if (elapsed < _speed*0.05)
    {
        rtn = NoteJudge::PERFECT;
    }
    else if(elapsed >=_speed*0.05 && elapsed < _speed*0.15)
    {
        rtn = NoteJudge::GREAT;

    }
    else if(elapsed >= _speed*0.15 && fabs(elapsed) < _speed*0.20)
    {
        rtn = NoteJudge::GOOD;
    }
    else if(elapsed >= _speed*0.20 && elapsed < _speed*0.50)
    {
        rtn = NoteJudge::BAD;
    }
    
    
    
    if(_isLongnote)
    {
        _longnotesHold = true;
    }
    else
    {
        removeFromParentAndCleanup(true);
        this->unscheduleUpdate();
    }
    
    return rtn;
}

NoteJudge Note::EndJudge()
{
    double now = StopWatch::getInstance()->currentTime();
    double offset = 5;
    double elapsed = fabs((_endTime-offset) - now);
    
    NoteJudge rtn;
    if (elapsed < _speed*0.05)
    {
        rtn = NoteJudge::PERFECT;
    }
    else if(elapsed >=_speed*0.05 && elapsed < _speed*0.15)
    {
        rtn = NoteJudge::GREAT;
        
    }
    else if(elapsed >= _speed*0.15 && fabs(elapsed) < _speed*0.20)
    {
        rtn = NoteJudge::GOOD;
    }
    else
    {
        rtn = NoteJudge::BAD;
    }
    

    removeFromParentAndCleanup(true);
    //Director::getInstance()->purgeCachedData();//無駄なテクスチャキャッシュは消去してメモリを押さえる
    this->unscheduleUpdate();
    
    //もしコールバックが存在するなら
    //if(_callbackFunc) _callbackFunc();
    
    return rtn;
}


void Note::update(float frame)
{

    double now = StopWatch::getInstance()->currentTime();
    double elapsed = now - startTimeCount;
    RenderTexture* note = this->getChildByName<RenderTexture*>("BaseNotes");

    cocos2d::Vec2 currentPos = note->getPosition();

    if(_isLongnote)
    {
        Sprite* sp = this->getChildByName<Sprite*>("EndNotes");
        if (now + _speed > _endTime)
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
//            DrawNode *innerPoly = getChildByName<DrawNode*>("innerLongnotesLine");
//            if(innerPoly == nullptr)
//            {
//                innerPoly = DrawNode::create();
//                innerPoly->setName("innerLongnotesLine");
//                poly->setBlendFunc((BlendFunc){GL_SRC_ALPHA, GL_ONE});
//                innerPoly->setPositionZ(poly->getPositionZ() - 3);
//                addChild(innerPoly);
//            }
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
            
//            innerPoly->clear();
//            innerPoly->drawPolygon(&vList[0], 4, Color4F(0.9,0.9, 0.5, opacity/1.2f),0, Color4F::BLACK);
//
//            innerPoly->setRotation(angle);
//            innerPoly->setPosition(_endOfPoint);
            
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
        }
        
        



        
        endTimeCount = now;
    }
    
    if(!_longnotesHold)
    {
        if(currentPos.x < 0 || currentPos.x > limitArea.width || currentPos.y < 0 || currentPos.y > limitArea.height)
        {
            if (_callbackFunc != nullptr)
            {
                _callbackFunc();
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
    
    startTimeCount = now;
}