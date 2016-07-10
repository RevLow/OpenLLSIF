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
#include <memory>
#include "Note.h"
USING_NS_CC;

enum GameLevel
{
    EASY,
    NORMAL,
    HARD,
    EXPERT
};

/*
 ゲームで使うファイルを引数にし、作成するためcreate_func<T>を実装する
 create_func<T>はPrefix.pchで定義しているため呼び出し可能
 */
class PlayScene : public Layer, create_func<PlayScene>
{
public:
    static cocos2d::Scene* createScene(std::string playSongFile, GameLevel level);
    bool init(std::string playSongFile, GameLevel level);
    using create_func::create;
    
    /**
     *@fn applicationWillEnterForeground
     *@brief バックグラウンドに入るときに呼ばれるメソッド
     */
    virtual void applicationDidEnterBackground();
    /**
     *@fn applicationWillEnterForeground
     *@brief バックグラウンドから復帰したときに呼ばれるメソッド
     */
    virtual void applicationWillEnterForeground();
    void finishCallBack(int audioID, std::string fileName);
private:
    int BGM_TIME;
    
    //! 全ノートの数
    int note_count;
    
    //
    int current_score;
    int max_score;
    //! ノーツのスピード
    int notesSpeed = 0;
    
    //! 楽曲のファイルパス
    std::string songFilePath;
    
    //! 楽曲譜面のデータを格納するためのベクター
    std::list< std::shared_ptr< std::queue<std::shared_ptr<cocos2d::ValueMap> > > > notesVector;
    
    //ノーツが進む方向の単位ベクトル
    std::vector<cocos2d::Vec2> unitVector;
    
    //各レーンごとに生成されているノーツキュー
    std::vector< std::queue<Note*> > createdNotes;
    
    
    //レイテンシーの値
    float latency = 0.0f;
    
    /**
     *@fn Run
     *@brief ゲームを開始するメソッド
     */
    void run();
    
    /**
     *@fn createNotes
     *ノーツ生成のための関数
     *@brief メインスレッドで指定レーンへのノーツを生成するメソッド
     *@param (notesNum) 生成するノーツ番号を格納したベクター
     */
    void createNotes(std::vector< std::shared_ptr<cocos2d::ValueMap> > maps);
    
    /**
     *  判定のスプライトを作成する
     *
     *  @param j 判定結果
     */
    void createJudgeSprite(NoteJudge j);
    
    /**
     *  タップのエフェクトを作成する
     *
     *  @param position 作成する起点となる場所
     */
    void createTapFx(Vec2 position);
    
    /**
     * 毎フレーム呼ばれるメソッド
     * @param dt 前のフレームから今のフレームまでにかかった時間
     */
    void update(float dt);
};

#endif /* defined(__OpenLLSIF__PlayScene__) */
