//
//  Note.h
//  OpenLLSIF
//
//  Created by RevLow on 2015/08/06.
//
//

#ifndef __OpenLLSIF__Note__
#define __OpenLLSIF__Note__

#include <iostream>
#include "cocos2d.h"
#include "StopWatch.h"


USING_NS_CC;

enum NoteJudge
{
    PERFECT = 0,
    GREAT,
    GOOD,
    BAD,
    MISS
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

//楕円形用のクラス
class Ellipse : public Node, create_func<Ellipse>
{
    CC_SYNTHESIZE(float, angle, Angle);
    CC_SYNTHESIZE(float, x_radius, XRadius);
    CC_SYNTHESIZE(float, y_radius, YRadius);
    CC_SYNTHESIZE(Point, m_obPosition, Point);
    
    using create_func::create;
public:
    virtual bool init(const Point p, float xr, float yr);
    void rotate(float theta);
    bool containsPoint(const Vec2 p);
    bool intersectCircle(Circle* circle);
    bool intersectEllipse(Ellipse* ellipse);
};

class Note : public Layer, create_func<Note>
{
    
public:
    using create_func::create;
    virtual bool init(ValueMap jsonInfo, cocos2d::Vec2 unitVec);
    NoteJudge StartJudge();
    NoteJudge EndJudge();
    
    void update(float flame);
    
    bool isLongNotes()
    {
        return _isLongnote;
    }
    bool isParallel()
    {
        return _isParallel;
    }
    //ノーツが消えるときに呼ばれるコールバック
    void setOutDisplayedCallback(const std::function<void()> &f);
    
    // 第何番目のレーンか
    unsigned int getLane()
    {
        return _lane;
    }
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
    //double _scaleTick;//msあたりのロングノーツの拡大スケール
    //double _totalStopTime = 0.0;
    double _startTime = 0.0;
    double _endTime = 0.0;
    double startTimeCount;
    double endTimeCount;
    std::function<void()> _callbackFunc;
};

#endif /* defined(__OpenLLSIF__Note__) */
