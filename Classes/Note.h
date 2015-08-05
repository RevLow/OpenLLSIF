//
//  Note.h
//  OpenLLSIF
//
//  Created by Tetsushi on 2015/08/06.
//
//

#ifndef __OpenLLSIF__Note__
#define __OpenLLSIF__Note__

#include <iostream>
#include "cocos2d.h"

USING_NS_CC;

class Note : public Sprite, create_func<Note>
{
public:
    using create_func::create;
    virtual bool init(ValueMap jsonInfo);
};

class NormalNote : Note
{
public:
    virtual bool init(ValueMap jsonInfo);
};

class LongRangeNote : Note
{
public:
    virtual bool init(ValueMap jsonInfo);
};

class StarNote : Note
{
public:
    virtual bool init(ValueMap jsonInfo);
};


#endif /* defined(__OpenLLSIF__Note__) */
