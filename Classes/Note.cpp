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
    _unitVec = unitVec;
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
    
    //ノーツ画像の作成
    note->begin();
    Sprite* baseNote;
    switch (type)
    {
        case 0:
            baseNote = Sprite::create("notes/smile_03.png");
            break;
        case 1:
            baseNote = Sprite::create("notes/cool_03.png");
            break;
        case 2:
            baseNote = Sprite::create("notes/pure_03.png");
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
         Sprite* parallel = Sprite::create("notes/longbar_03.png");
         parallel->retain();
         parallel->setPosition(35.0f, 35.0f);
         //スター画像を書き込み
         parallel->visit();
         CC_SAFE_RELEASE(parallel);
     }
    
     if(_isStar)
     {
         Sprite* star = Sprite::create("notes/star_03.png");
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
    
    note->setPosition(479.75, 476.34);
    note->setScale(0.3f);
    note->runAction(EaseIn::create(cocos2d::ScaleTo::create(_speed / 1000.0, 2.0f), 1.3f));
    
    if(_isLongnote)
    {
        _endOfPoint = Vec2(480,480);
        //キャッシュにテクスチャを登録
        Sprite* sp = Sprite::create("notes/longnotes_03.png");
        sp->setName("EndNotes");
        sp->setVisible(false);
        sp->setPosition(_endOfPoint);
        sp->setScale(0.3f);
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
    double elapsed = fabs(_startTime - now);
    
    NoteJudge rtn;
    if (elapsed < 45)
    {
        rtn = NoteJudge::PERFECT;
    }
    else if(elapsed >=45 && elapsed < 120)
    {
        rtn = NoteJudge::GREAT;

    }
    else if(elapsed >= 120 && fabs(elapsed) < 200)
    {
        rtn = NoteJudge::GOOD;
    }
    else
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
    double elapsed = fabs(_endTime - now);
    
    NoteJudge rtn;
    if (elapsed < 45)
    {
        rtn = NoteJudge::PERFECT;
    }
    else if(elapsed >=45 && elapsed < 120)
    {
        rtn = NoteJudge::GREAT;
        
    }
    else if(elapsed >= 120 && fabs(elapsed) < 200)
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
    if(_callbackFunc) _callbackFunc();
    
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
            sp->setVisible(true);
            sp->setPosition(_endOfPoint);
            
            auto runningAction = sp->getActionByTag(1);
            
            if(!runningAction && now < _endTime)
            {
                auto action = EaseIn::create(cocos2d::ScaleTo::create(_speed / 1000.0, 2.0f), 1.3f);
                action->setTag(1);
                sp->runAction(action);
                
            }
        }


        Vec2 v1 = currentPos - Vec2(480,480);
        Vec2 v2(0, -1);
        
        double length = (currentPos - _endOfPoint).length();
        float angle = MATH_RAD_TO_DEG(v1.getAngle(v2));


        length += 25;
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

        
        
        FilledPolygon *poly = getChildByName<FilledPolygon*>("LongnotesLine");
        
        if (poly == nullptr)
        {
            Texture2D *texture = Director::getInstance()->getTextureCache()->addImage("notes/longNoteLine_07.png");
            
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