//
//  PlayScene.h
//  OpenLLSIF
//
//  Created by RevLow on 2015/07/20.
//
//

#ifndef __OpenLLSIF__PlayScene__
#define __OpenLLSIF__PlayScene__

#include <iostream>

USING_NS_CC;

/*
 ゲームで使うファイルを引数にし、作成するためcreate_func<T>を実装する
 create_func<T>はPrefix.pchで定義しているため呼び出し可能
 */
class PlayScene : public Layer, create_func<PlayScene>
{
public:
    static cocos2d::Scene* createScene(std::string playSongFile);
    bool init(std::string playSongFile);
    using create_func::create;
private:
    int BPM;        //音楽のBPM
    int BGM_TIME;   //音楽の時間
    int NOTE_COUNT; //全ノートの数
    void Run();
    void PlayGame(float deltaT);
};

#endif /* defined(__OpenLLSIF__PlayScene__) */
