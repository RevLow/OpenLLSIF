//
//  PlayScene.h
//  OpenLLSIF
//
//  Created by RevLow on 2015/07/20.
//
//

#ifndef __OpenLLSIF__PlayScene__
#define __OpenLLSIF__PlayScene__

#include <memory>
#include <unordered_map>
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
    bool init(std::string playSongFile, GameLevel gameLevel);
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
    
    /**
     *  BGMの再生が終了したときに呼ばれるコールバック
     *
     *  @param audioID
     *  @param fileName
     */
    void finishCallBack();
    
    //ノーツに関するコールバック関数
    
    /**
     *  ノーツが画面判定外に出たときに呼ばれるコールバック関数
     *
     *  @param note
     */
    void noteOutDisplayedCallback(const Note& note);
    
    /**
     *  ノーツがタップされたときに呼ばれるコールバック関数
     *
     *  @param note
     */
    void noteTouchCallback(const Note& note);
    
    /**
     *  ノーツを離したときに呼ばれるコールバック関数
     *
     *  @param note 
     */
    void noteReleaseCallback(const Note& note);
    
    void onTouchesBegan(const std::vector<Touch*>& touches, Event *event);
    void onTouchesMoved(const std::vector<Touch*>& touches, Event *event);
    void onTouchesEnded(const std::vector<Touch*>& touches, Event *event);
private:
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
    void createNotes(const ValueMap& maps);
    
    /**
     *  判定のスプライトを作成する
     *
     *  @param j 判定結果
     */
    void createJudgeSprite(NoteJudge judge);
    
    /**
     *  タップのエフェクトを作成する
     *
     *  @param position 作成する起点となる場所
     */
    void createTapFx(Vec2 position);
    
    /**
     *  ゲーム開始前に音声のプリロード処理などを行うためのメソッド
     */
    void prepareGameRun();
    
    
    /**
     * 毎フレーム呼ばれるメソッド
     * @param dt 前のフレームから今のフレームまでにかかった時間
     */
    void update(float dt);
private:
    //! 現在のスコア
    int _currentScore;
    
    //! 計算で算出される最大スコア
    int _maxScore;
    
    //! ノーツのスピード
    int _notesSpeedMs = 0;
    
    //! 楽曲のファイルパス
    std::string _songFilePath;
    
    
    typedef std::shared_ptr<ValueMap> ValueMapPtr;
    
    //! 楽曲譜面のデータを格納するためのベクター
    std::list< std::queue<ValueMapPtr>  > _notesVector;
    
    //! ノーツが進む方向の単位ベクトル
    std::vector<cocos2d::Vec2> _directionUnitVector;
    
    //! 各レーンごとに生成されているノーツキュー
    std::vector< std::queue<Note*> > _displayedNotes;
    
    std::unordered_map<int, Note*> _holdNotes;
    
    //! レイテンシーの値
    float _latencyMs = 0.0f;
};

#endif /* defined(__OpenLLSIF__PlayScene__) */
