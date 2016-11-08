//
//  SifUtil.cpp
//  OpenLLSIF
//
//  Created by RevLow on 2016/11/08.
//
//

#include "SifUtil.h"

float SifUtil::decreaseEasing(float t)
{
    return 1.9975 * pow(t, 5) - 0.747499999999999 * pow(t, 4) - 2.3 * pow(t, 3) + 0.05 * t + 1;
}

float SifUtil::increaseEasing(float t)
{
    return 1.7*pow(t, 5) -6 * pow(t, 4) + 8.8 * pow(t, 3) + -7.4 * pow(t, 2) + 3.9 * t;
}

Vec2 SifUtil::unitPosition(int unitNum)
{
    float radian = MATH_DEG_TO_RAD(22.5 * unitNum);
    return std::move(Vec2(SifUtil::initVec.x - cos(radian) * 400.0,
                          SifUtil::initVec.y - sin(radian) * 400.0));
}