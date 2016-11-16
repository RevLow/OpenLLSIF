//
//  SifUtil.h
//  OpenLLSIF
//
//  Created by RevLow on 2016/11/08.
//
//

#ifndef __OpenLLSIF__SifUtil__
#define __OpenLLSIF__SifUtil__

#include "cocos2d.h"

USING_NS_CC;

namespace SifUtil
{
    //! ノーツが出現する位置
    const Vec2 initVec(480, 480);

    //! ユニットを配置する半径
    const float UNIT_RADIUS = 400;
    
    //! ユニット間の角度
    const float BETWEEN_UNITS_ANGLE = 22.5;
    
    /**
     *  減少のイージング
     *
     *  @param float 時間
     *
     *  @return 入力時間のときの値(0.0 ~ 1.0)
     */
    float decreaseEasing(float);
    
    /**
     *  増加のイージング
     *
     *  @param float 時間
     *
     *  @return 入力時間の時の値(0.0 ~ 1.0)
     */
    float increaseEasing(float);
    
    /**
     *  Unit画像の位置情報を取得する
     *
     *  @param unitNum 取得するユニットの位置(0 ~ 8)
     *
     *  @return ユニットの位置
     */
    Vec2 unitPosition(int unitNum);
}

#endif /* defined(__OpenLLSIF__SifUtil__) */
