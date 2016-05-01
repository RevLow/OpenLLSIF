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
    //タップ判定用のエリア
    std::vector< Vector<Circle*> > expandedAreas;
    
    //各レーンごとに生成されているノーツキュー
    std::vector< std::queue<Note*> > createdNotes;
    
    //タップされたロングノーツと押している指の対応付け
    cocos2d::Map<int, Note*> _longNotes;
    
    //レイテンシーの値
    float latency = 0.0f;
    
    /**
     *@fn Run
     *@brief ゲームを開始するメソッド
     */
    void Run();
    
    /**
     *@fn CreateNotes
     *ノーツ生成のための関数
     *@brief メインスレッドで指定レーンへのノーツを生成するメソッド
     *@param (notesNum) 生成するノーツ番号を格納したベクター
     */
    void CreateNotes(std::vector< std::shared_ptr<cocos2d::ValueMap> > maps);
    
    void CreateJudgeSprite(NoteJudge j);
    
    void CreateTapFx(Vec2 position);
    //タップイベント
    void onTouchesBegan(const std::vector<Touch *> &touches, cocos2d::Event *unused_event);
    void onTouchesEnded(const std::vector<Touch *> &touches, cocos2d::Event *unused_event);
    void update(float dt);
};

#endif /* defined(__OpenLLSIF__PlayScene__) */
