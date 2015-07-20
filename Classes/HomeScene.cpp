//
//  HomeScene.cpp
//  OpenLLSIF
//
//  Created by RevLow on 2015/07/13.
//
//

#include "HomeScene.h"
#include "ui/cocosGui.h"
#include "cocostudio/CocoStudio.h"
#include "SimpleAudioEngine.h"
#include <dirent.h>
#include "PointWithDepth.h"

USING_NS_CC;


/*
 指定ディレクトリ内のファイルのリストを取得する関数
 */
std::vector<std::string> getDirContents(std::string dirname) {
    FileUtils* fu = FileUtils::getInstance();
    
    std::string dir = fu->fullPathForFilename(dirname);
    
    std::vector<std::string> list;
    
    DIR* dp;
    struct dirent* ent;
    
    if ((dp = opendir(dir.c_str()), "r") == NULL) {
        CCLOGERROR("ディレクトリが開けません。：%s", dir.c_str());
        perror(dir.c_str());
        return list;
    }
    
    while ((ent = readdir(dp)) != NULL) {
        CCLOG("ファイル：%s", ent->d_name);
        if(ent->d_type != '\x04')
        {
            list.push_back(ent->d_name);
        }
        
    };
    closedir(dp);
    
    return list;
}




Scene* HomeScene::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();
    
    // 'layer' is an autorelease object
    auto layer = HomeScene::create();
    
    // add layer as a child to scene
    scene->addChild(layer);
    
    // return the scene
    return scene;
}

bool HomeScene::init()
{
    if (!Layer::init()) {
        return false;
    }
    
    //背景レイヤーの作成
    //ここの階層を変えていく
    auto background = CSLoader::getInstance()->createNode("res/home_background.csb");
    background->setName("backgroundLayer");
    auto action = cocostudio::timeline::ActionTimelineCache::getInstance()->createAction("res/home_background.csb");
    
    //背景レイヤーのアニメーションを実行
    background->runAction(action);
    action->gotoFrameAndPlay(0, true);
    //明示的にz座標を指定
    background->setLocalZOrder(0);
    addChild(background);
    
    //ボタンのレイヤーを作成
    auto homeScene = CSLoader::getInstance()->createNode("res/HomeScene.csb");
    homeScene->setName("HomeSceneLayer");
    homeScene->setLocalZOrder(1);
    action = cocostudio::timeline::ActionTimelineCache::getInstance()->createAction("res/HomeScene.csb");
    
    homeScene->runAction(action);
    //最初を0フレーム目まで移動させとく
    action->gotoFrameAndPause(0);
    
    addChild(homeScene);
    
    
    //各種ボタンの設定
    //ホームボタンの設定
    auto homeButton = homeScene->getChildByName<ui::Button*>("Home_Button");
    homeButton->addClickEventListener(CC_CALLBACK_1(HomeScene::homeButton_action, this));
    
    //ライブボタンの設定
    auto liveButton = homeScene->getChildByName<ui::Button*>("Live_Button");
    liveButton->addClickEventListener(CC_CALLBACK_1(HomeScene::liveButton_action, this));
    
    
    return true;
}

/*
 シーンの読み込みが完了したときにこの関数を実行する
 */
void HomeScene::onEnterTransitionDidFinish()
{
    
    auto homeScene = this->getChildByName("HomeSceneLayer");
    auto action = cocostudio::timeline::ActionTimelineCache::getInstance()->createAction("res/HomeScene.csb");
    
    homeScene->runAction(action);
    action->gotoFrameAndPlay(0, false);
    CocosDenshion::SimpleAudioEngine::getInstance()->preloadBackgroundMusic("Sound/BGM/title_bgm.mp3");
    CocosDenshion::SimpleAudioEngine::getInstance()->playBackgroundMusic("Sound/BGM/title_bgm.mp3", true);

}

/*
 ホームボタンを押したときの処理
 背景をホームの背景に変更を行い、アニメーションを再生する
 */
void HomeScene::homeButton_action(Ref *ref)
{
    
    //ボタンを押したときの動作を設定
    auto background = this->getChildByName("backgroundLayer");


    background->runAction(Sequence::create(FadeTo::create(0.3f, 0),CallFunc::create([&]()
                                                                                    {
                                                                                        this->removeChildByName("backgroundLayer");
                                                                                        this->removeChildByName("jacketLayer");
                                                                                        auto newLayer = CSLoader::getInstance()->createNode("res/home_background.csb");
                                                                                        auto action = cocostudio::timeline::ActionTimelineCache::getInstance()->createAction("res/home_background.csb");
                                                                                        newLayer->setName("backgroundLayer");
                                                                                        
                                                                                        newLayer->runAction(action);
                                                                                        action->gotoFrameAndPlay(0, true);
                                                                                        this->addChild(newLayer, 0);
                                                                                    }), NULL));

    CocosDenshion::SimpleAudioEngine::getInstance()->preloadEffect("Sound/SE/decide.mp3");
    CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("Sound/SE/decide.mp3");
    
}



/*
ライブボタンを押したときの処理
背景をliveの背景に変更を行い、擬似的な3次元空間上にアルバムのジャケットを追加する
また、ボタンにはアクションを追加し、アクションはまた別の関数で定義を行っている
 */
void HomeScene::liveButton_action(Ref *ref)
{
    //ボタンを押したときの動作を設定
    auto background = this->getChildByName("backgroundLayer");
    
    background->runAction(Sequence::create(FadeTo::create(0.3f, 0),CallFunc::create([this]()
                                                                                    {
                                                                                        this->removeChildByName("backgroundLayer");
                                                                                        
                                                                                        
                                                                                        
                                                                                        auto newLayer = CSLoader::getInstance()->createNode("res/song_selection.csb");
                                                                                        auto action = cocostudio::timeline::ActionTimelineCache::getInstance()->createAction("res/song_selection.csb");
                                                                                        newLayer->setName("backgroundLayer");
                                                                                        
                                                                                        newLayer->runAction(action);
                                                                                        action->gotoFrameAndPlay(0,true);
                                                                                        this->addChild(newLayer, 0);
                                                                                        
                                                                                        
                                                                                        //ジャケットを配置するノードの作成
                                                                                        //実際に使うときはzipファイル内から画像を抽出し、それを配置していく
                                                                                        //ジャケットサイズは今後考える
                                                                                        Node* jacketNode = Node::create();
                                                                                        jacketNode->setName("jacketLayer");
                                                                                        
                                                                                        
                                                                                        
                                                                                        //Jacketディレクトリからファイルのリストを取得
                                                                                        auto fileList = getDirContents("jacket");
                                                                                        float theta = 0;
                                                                                        float dTheta = 360 / fileList.size();
                                                                                        float z = 0;
                                                                                        
                                                                                        
                                                                                        for(int i=0; i < fileList.size();i++)
                                                                                        {
                                                                                            auto img = fileList[i];
                                                                                            auto *sp = Sprite::create("jacket/"+img);
                                                                                            //あらかじめタグ付けを行っておく事で回転のボタンを押したときの処理を行えるようにする
                                                                                            sp->setTag(i);
                                                                                            sp->setName("jacket/"+img);
                                                                                            
                                                                                            
                                                                                            double rad = MATH_DEG_TO_RAD(theta);
                                                                                            Vec3 v(480*sin(rad), 0.0, 4.8*cos(rad));
                                                                                            PointWithDepth point;
                                                                                            point.SetWorldPosition(v.x, v.y, v.z);
                                                                                            
                                                                                            
                                                                                            sp->setPosition(point);
                                                                                            sp->setScale(point.GetScale());
                                                                                            
                                                                                            jacketNode->addChild(sp,z);
                                                                                            
                                                                                            
                                                                                            //次にaddChildする位置を決める
                                                                                            if(i < fileList.size() / 2) z--;
                                                                                            else if(i==fileList.size()/2) z = fileList.size()%2 ? z: z+1;
                                                                                            else z++;
                                                                                            
                                                                                            theta += dTheta;
                                                                                        }
                                                                                        
                                                                                        
                                                                                        Sprite* backgroundSprite = Sprite::create("res/Image/hard_background.png");
                                                                                        backgroundSprite->setName("backgroundImage");
                                                                                        backgroundSprite->setPosition(480, 320);
                                                                                        backgroundSprite->setLocalZOrder(-2);
                                                                                        this->addChild(backgroundSprite);
                                                                                        
                                                                                        jacketNode->setLocalZOrder(-1);
                                                                                        jacketNode->setPosition(480, 380);
                                                                                        this->addChild(jacketNode);
                                                                                        
                                                                                        
                                                                                        
                                                                                        //ボタンの設定
                                                                                        auto nextButton = newLayer->getChildByName<ui::Button*>("next_button");
                                                                                        nextButton->addClickEventListener(CC_CALLBACK_1(HomeScene::nextAlbum_click, this));
                                                                                        
                                                                                        auto previousButton = newLayer->getChildByName<ui::Button*>("previous_button");
                                                                                        previousButton->addClickEventListener(CC_CALLBACK_1(HomeScene::previousAlbum_click, this));
                                                                                        
                                                                                        
                                                                                        //jacketNodeにイベントリスナーを追加する
                                                                                        auto listener = EventListenerTouchOneByOne::create();
                                                                                        //listener->setSwallowTouches(true);
                                                                                        listener->onTouchBegan = CC_CALLBACK_2(HomeScene::jacket_touch, this);
                                                                                        jacketNode->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, this);
                                                                                        
                                                                                    }), NULL));
    
    CocosDenshion::SimpleAudioEngine::getInstance()->preloadEffect("Sound/SE/decide.mp3");
    CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("Sound/SE/decide.mp3");
}


/*
 live画面中での次のボタンを押したときのジャケットに対する処理
 addChildするときに指定したタグをもとに円形に配置したとなりのジャケットを参照、そして移動、スケールを繰り返す
 */
void HomeScene::nextAlbum_click(Ref *ref)
{
    
    CocosDenshion::SimpleAudioEngine::getInstance()->preloadEffect("Sound/SE/selection.mp3");
    CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("Sound/SE/selection.mp3");
    
    
    auto jacketNode = this->getChildByName("jacketLayer");
    
    //クリック可能なスプライトを変更する
    touchable_index = touchable_index == 0 ? jacketNode->getChildrenCount() - 1 : touchable_index-1;
    
    //ループを行う事で最初の状態は変化してしまうためあらかじめ保持しておく
    int zeroLocalZ = jacketNode->getChildByTag(0)->getLocalZOrder();
    float zeroScale = jacketNode->getChildByTag(0)->getScale();
    Point zeroPoint = jacketNode->getChildByTag(0)->getPosition();
    
    //ループは最後以外を行い、最後はループを抜けた後行う
    for(int i=0;i<jacketNode->getChildrenCount() - 1;i++)
    {
        auto node = jacketNode->getChildByTag(i);
        Node* nstNode = jacketNode->getChildByTag(i+1);
        
        node->runAction(Sequence::create(Spawn::create(MoveTo::create(0.2f, nstNode->getPosition()), ScaleTo::create(0.2f, nstNode->getScale()), NULL),
                                         CallFunc::create([node, nstNode]()
                                                          {
                                                              node->setLocalZOrder(nstNode->getLocalZOrder());
                                                          })
                                         , NULL)
                        
                        );
    }
    //最後のノードは最初のノードの位置に行くため、保持しておいた情報をもとに変化させる
    auto finalNode = jacketNode->getChildByTag(jacketNode->getChildrenCount()-1);
    finalNode->runAction(Sequence::create(Spawn::create(MoveTo::create(0.2f, zeroPoint), ScaleTo::create(0.2f, zeroScale), NULL),
                                          CallFunc::create([finalNode, zeroLocalZ]
                                                           {
                                                               finalNode->setLocalZOrder(zeroLocalZ);
                                                           })
                                          , NULL));
}

/*
 live画面中での前のボタンを押したときのジャケットに対する処理
 addChildするときに指定したタグをもとに円形に配置したとなりのジャケットを参照、そして移動、スケールを繰り返す
 */
void HomeScene::previousAlbum_click(Ref *ref)
{
    CocosDenshion::SimpleAudioEngine::getInstance()->preloadEffect("Sound/SE/selection.mp3");
    CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("Sound/SE/selection.mp3");
    
    auto jacketNode = this->getChildByName("jacketLayer");
    
    //クリック可能なスプライトを変更する
    touchable_index = touchable_index == jacketNode->getChildrenCount() - 1 ? 0 : touchable_index+1;
    
    //ループを行う事で最初の状態は変化してしまうためあらかじめ保持しておく
    int zeroLocalZ = jacketNode->getChildByTag(jacketNode->getChildrenCount() - 1)->getLocalZOrder();
    float zeroScale = jacketNode->getChildByTag(jacketNode->getChildrenCount() - 1)->getScale();
    Point zeroPoint = jacketNode->getChildByTag(jacketNode->getChildrenCount() - 1)->getPosition();
    
    //ループは最後以外を行い、最後はループを抜けた後行う
    for(int i=jacketNode->getChildrenCount() - 1;i>0;i--)
    {
        auto node = jacketNode->getChildByTag(i);
        Node* nstNode = jacketNode->getChildByTag(i-1);
        
        node->runAction(Sequence::create(
                                         Spawn::create(MoveTo::create(0.2f, nstNode->getPosition()), ScaleTo::create(0.2f, nstNode->getScale()), NULL),
                                         CallFunc::create([node, nstNode]()
                                                        {
                                                            node->setLocalZOrder(nstNode->getLocalZOrder());
                                                        })
                                         , NULL)
                        );
    }
    //最後のノードは最初のノードの位置に行くため、保持しておいた情報をもとに変化させる
    auto finalNode = jacketNode->getChildByTag(0);
    finalNode->runAction(Sequence::create(Spawn::create(MoveTo::create(0.2f, zeroPoint), ScaleTo::create(0.2f, zeroScale), NULL),
                                          CallFunc::create([finalNode, zeroLocalZ]
    {
        finalNode->setLocalZOrder(zeroLocalZ);
    })
                                          , NULL));
    
}

/*
 ジャケットをクリックしたときの処理を行う
 touchable_indexとクリックされたスプライトのタグ番号を比較し、等しかったらゲームのシーンに移行する
 参考: http://ladywendy.com/lab/cocos2d-x-v3/170.html
 */
bool HomeScene::jacket_touch(cocos2d::Touch* touch, cocos2d::Event* e)
{
    Node* node = this->getChildByName("jacketLayer");
    
    //クリックされたターゲットを取得する
    //auto target = (Sprite*)e->getCurrentTarget();
    
    //クリックされたスプライトの領域
    auto referenceSprite = (Sprite*)node->getChildByTag(touchable_index);
    
    Rect targetBox = referenceSprite->getBoundingBox();
    
    //クリックされた位置を取得
    Vec2 touchPoint = node->convertTouchToNodeSpace(touch);
    
    if(targetBox.containsPoint(touchPoint))
    {
        //決定音を鳴らす
        CocosDenshion::SimpleAudioEngine::getInstance()->preloadEffect("Sound/SE/decide2.mp3");
        CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("Sound/SE/decide2.mp3");
        
        //ノードの数が減っていくためあらかじめ数を保持しておく
        int num = node->getChildrenCount();
        
        //選択項目以外を削除
        for(int i=0;i < num;i++)
        {
            auto sp = node->getChildByTag(i);
            if(sp != referenceSprite)
            {
                node->removeChild(sp);
            }
        }
        
        //選択項目をアニメーション
        auto seqAction = Sequence::create(Spawn::create(
                                                        ScaleTo::create(0.5f, 1.5),
                                                        FadeTo::create(0.5f, 0)
                                                        , NULL),
                                          CallFunc::create([node,referenceSprite]()
                                                            {
                                                                //アニメーション終了後にreferenceSpriteを消去
                                                                node->removeChild(referenceSprite);
                                                            }), NULL);
        referenceSprite->runAction(seqAction);
        
        //ここで次のシーンへの遷移を行う
        
        return true;
    }
    
    //touchPointが範囲内に入っていれば
    return false;
}