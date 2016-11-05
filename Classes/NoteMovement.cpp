//
//  NoteMovement.cpp
//  OpenLLSIF
//
//  Created by RevLow on 2016/11/05.
//
//

#include "NoteMovement.h"

NoteMovement::NoteMovement()
{
    
}

NoteMovement::~NoteMovement()
{
    
}

NoteMovement* NoteMovement::create(const cocos2d::Vec2 &direction)
{
    NoteMovement* ret = new (std::nothrow) NoteMovement();
    if (ret && ret->init(direction))
    {
        ret->autorelease();
        return ret;
    }
    
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool NoteMovement::init(const cocos2d::Vec2 &direction)
{
    _direction = direction;
    return true;
}

NoteMovement* NoteMovement::clone() const
{
    return NoteMovement::create(_direction);
}

NoteMovement* NoteMovement::reverse() const
{
    return NoteMovement::create(-_direction);
}

void NoteMovement::step(float dt)
{
    this->update(dt);
}

void NoteMovement::update(float time)
{
    if (_target)
    {
        Vec2 currentPosition = _target->getPosition();
        currentPosition += _direction * time;
        _target->setPosition(currentPosition);
    }
}