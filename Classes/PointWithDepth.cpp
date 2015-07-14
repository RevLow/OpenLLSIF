//
//  PointWithDepth.cpp
//  OpenLLSIF
//
//  Created by RevLow on 2015/07/14.
//
//

#include "PointWithDepth.h"

void PointWithDepth::SetWorldPosition(float local_x, float local_y, float local_z)
{
    m_scale=(Z_S-Z_E)/(local_z-Z_E);
    cocos2d::Point::x=X_INF+m_scale*local_x;
    cocos2d::Point::y=Y_INF+m_scale*local_y;
}

float PointWithDepth::GetScale()
{
    return m_scale;
}

