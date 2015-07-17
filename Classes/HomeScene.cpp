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
                                                                                        
                                                                                        
                                                                                        
                                                                                        jacketNode->setLocalZOrder(-1);
                                                                                        jacketNode->setPosition(480, 380);
                                                                                        this->addChild(jacketNode);
                                                                                        
                                                                                        
                                                                                        
                                                                                        //ボタンの設定
                                                                                        auto nextButton = newLayer->getChildByName<ui::Button*>("next_button");
                                                                                        nextButton->addClickEventListener(CC_CALLBACK_1(HomeScene::nextAlbum_click, this));
                                                                                        
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
    auto jacketNode = this->getChildByName("jacketLayer");
    
    //ループを行う事で最初の状態は変化してしまうためあらかじめ保持しておく
    int zeroLocalZ = jacketNode->getChildByTag(0)->getLocalZOrder();
    float zeroScale = jacketNode->getChildByTag(0)->getScale();
    Point zeroPoint = jacketNode->getChildByTag(0)->getPosition();
    
    //ループは最後以外を行い、最後はループを抜けた後行う
    for(int i=0;i<jacketNode->getChildrenCount() - 1;i++)
    {
        auto node = jacketNode->getChildByTag(i);
        Node* nstNode = jacketNode->getChildByTag(i+1);
        
        node->runAction(Spawn::create(MoveTo::create(0.3f, nstNode->getPosition()), ScaleTo::create(0.3f, nstNode->getScale()), NULL));
        node->setLocalZOrder(nstNode->getLocalZOrder());
    }
    //最後のノードは最初のノードの位置に行くため、保持しておいた情報をもとに変化させる
    auto finalNode = jacketNode->getChildByTag(jacketNode->getChildrenCount()-1);
    finalNode->runAction(Spawn::create(MoveTo::create(0.3f, zeroPoint), ScaleTo::create(0.3f, zeroScale), NULL));
    finalNode->setLocalZOrder(zeroLocalZ);
}