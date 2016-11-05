//
//  NoteMovement.h
//  OpenLLSIF
//
//  Created by RevLow on 2016/11/05.
//
//

#ifndef __OpenLLSIF__NoteMovement__
#define __OpenLLSIF__NoteMovement__

#include <cocos2d.h>

USING_NS_CC;

/**
 *  特定方向に無限に移動させるアクション
 */
class NoteMovement : public FiniteTimeAction
{
public:
    /**
     *  Create Action
     *
     *  @param direction 毎秒移動させる方向
     *
     *  @return NoteMovement Action
     */
    static NoteMovement* create(const Vec2& direction);
    
    virtual void step(float dt) override;
    virtual void update(float time) override;
    virtual NoteMovement* clone() const override;
    virtual NoteMovement* reverse() const override;
    virtual bool isDone() const { return false; }

CC_CONSTRUCTOR_ACCESS:
    NoteMovement();
    ~NoteMovement();
    
    bool init(const Vec2& direction);
private:
    Vec2 _direction;

    // コピーコンストラクタと代入演算子を削除
    CC_DISALLOW_COPY_AND_ASSIGN(NoteMovement);
};

#endif /* defined(__OpenLLSIF__NoteMovement__) */
