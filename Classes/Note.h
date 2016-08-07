//
//  Note.h
//  OpenLLSIF
//
//  Created by RevLow on 2015/08/06.
//
//

#ifndef __OpenLLSIF__Note__
#define __OpenLLSIF__Note__

#include "cocos2d.h"
#include "StopWatch.h"

USING_NS_CC;



enum NoteJudge
{
    PERFECT = 0,
    GREAT,
    GOOD,
    BAD,
    MISS,
    NON
};

//円を作るクラス 判定のために使用
class Circle : public Node, create_func<Circle>
{
    CC_SYNTHESIZE(float, radius , Radius);
    CC_SYNTHESIZE(Point, m_obPosition, Point);
    
    using create_func::create;
public:
    virtual bool init(const Point p, float radius);
    bool containsPoint(const Vec2 p);
    bool intersectRect(const Rect rect);
    bool intersectCircle(Circle* circle);
    DrawNode* getDrawNode(Color4F c);
};


class Note : public Layer, create_func<Note>
{
    
public:
    using create_func::create;
    virtual bool init(const ValueMap& jsonInfo, cocos2d::Vec2 unitVec);

    void update(float flame);
    
    const bool isLongNotes() const
    {
        return _isLongnote;
    }
    const bool isParallel() const
    {
        return _isParallel;
    }
    //ノーツが消えるときに呼ばれるコールバック
    void setOutDisplayedCallback(const std::function<void(const Note&)> &f);
    
    //ノーツをタップしたときのコールバック
    void setTouchCallback(const std::function<void(const Note&)> &f);

    //ノーツを離したときのコールバック
    void setReleaseCallback(const std::function<void(const Note&)> &f);
    
    // 第何番目のレーンか
    const unsigned int getLane() const
    {
        return _lane;
    }
    
    //タップイベント
    bool touchBeginAction(int touch_id);
    void touchEndAction(int touch_id);
    void touchMoveAction(int touch_id);
    
    CC_SYNTHESIZE_READONLY(NoteJudge, _judgeResult, JudgeResult);
private:
    void createNotesSprite(Vec2 &initVec, int type);
    //bool isPointContain(Vec2 pos);
    NoteJudge startJudge();
    NoteJudge endJudge();
private:
    double _speed;
    bool _isStar = false;//星付きか
    bool _isParallel = false;//複数レーンか
    bool _isLongnote = false;
    
    bool _longnotesHold = false;//ロングノーツをつかんでいるか
    unsigned int _lane = 0;//レーン番号、左上から右上まで順番に0~8
    cocos2d::Vec2 _unitVec;//1フレームあたりの増加量
    cocos2d::Vec2 _endOfPoint;
    cocos2d::Size limitArea;//画面範囲
    cocos2d::Vec2 _destination;//目的地
    //double _scaleTick;//msあたりのロングノーツの拡大スケール
    //double _totalStopTime = 0.0;
    double _startTime = 0.0;
    double _endTime = 0.0;
    double startTimeCount;
    double endTimeCount;
    
    float latency = 0.0f;
    int _longNotesHoldId;
    std::function<void(const Note&)> _callbackFunc;
    std::function<void(const Note&)> _touchCallbackFunc;
    std::function<void(const Note&)> _releaseCallbackFunc;
};
#endif /* defined(__OpenLLSIF__Note__) */
