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
#include "PRFilledPolygon.h"

USING_NS_CC;

/**
 *  減少のイージング
 *
 *  @param float 時間
 *
 *  @return 入力時間のときの値(0.0 ~ 1.0)
 */
inline float decreaseEasing(float);

/**
 *  増加のイージング
 *
 *  @param float 時間
 *
 *  @return 入力時間の時の値(0.0 ~ 1.0)
 */
inline float increaseEasing(float);

enum NoteJudge
{
    PERFECT = 0,
    GREAT,
    GOOD,
    BAD,
    MISS,
    NON
};

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
        return _noteInfo.isLongNote;
    }
    const bool isParallel() const
    {
        return _noteInfo.isParallel;
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
        return _noteInfo.lane;
    }
    
    //タップイベント
    bool touchBeginAction(int touch_id);
    void touchEndAction(int touch_id);
    void touchMoveAction(int touch_id);
    
    CC_SYNTHESIZE_READONLY(NoteJudge, _judgeResult, JudgeResult);
private:
    void createNotesSprite(Vec2 &initVec, int type);
    NoteJudge startJudge();
    NoteJudge endJudge();
    
    void updateSimpleNote(double elapsed, Sprite* note);
    void updateLongNote(double elapsed, Sprite* note);
    void flickerPolygon(FilledPolygon* poly, double sleepTime);
    void renderFilledPolygon(Sprite* startNoteSprite, Sprite* endNoteSprite);
    inline void updateNotePosition(Sprite* note, float elapsedTime);
private:
    //点滅の状態
    struct FlickerState
    {
        bool isIncrease;
        float time;
        float duration;
        std::function<float(float)> easing;

        FlickerState();
    };
    FlickerState _lnFlash;
    
    //ノートの情報
    struct NoteInfo
    {
        unsigned int lane;
        
        double startTime;
        double endTime;
        double speed;
        
        bool isStar;
        bool isParallel;
        bool isLongNote;
        bool isHolding;
        
        //! ノーツの進む方向
        Vec2 direction;
        
        NoteInfo();
    };
    NoteInfo _noteInfo;

    Vec2 _endOfPoint;
    Vec2 _destination;//目的地
    
    float latency = 0.0f;
    float _elapsedTime;
    int _longNotesHoldId;
    Rect _visibleRect;
    
    
    std::function<void(const Note&)> _callbackFunc;
    std::function<void(const Note&)> _touchCallbackFunc;
    std::function<void(const Note&)> _releaseCallbackFunc;
};
#endif /* defined(__OpenLLSIF__Note__) */
