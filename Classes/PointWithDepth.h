//
//  PointWithDepth.h
//  OpenLLSIF
//
//  Created by RevLow on 2015/07/14.
//
// Cocos2d-xで2.5次元表現するためのクラス
//参考: http://hackerslab.aktsk.jp/technology/expressing-depth-with-cocos2d-x-jp/

#ifndef __OpenLLSIF__PointWithDepth__
#define __OpenLLSIF__PointWithDepth__

#include <iostream>

class PointWithDepth:public cocos2d::Point
{
    const float Z_S =5;
    const float Z_E =Z_S + 10;
    //消失点の位置
    const float X_INF = 0;
    const float Y_INF = 0;
public:
    void SetWorldPosition(float local_x,float local_y,float local_z);
    float GetScale(void);
private:
    float m_scale;
};

#endif /* defined(__OpenLLSIF__PointWithDepth__) */
