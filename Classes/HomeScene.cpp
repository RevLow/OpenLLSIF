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
    homeButton->addClickEventListener([this](Ref *ref)
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
                                            
                                        });
    
    //ライブボタンの設定
    auto liveButton = homeScene->getChildByName<ui::Button*>("Live_Button");
    liveButton->addClickEventListener([this](Ref *ref)
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
                                                                                                                                  //auto *sp = BillBoard::create("jacket/"+img,BillBoard::Mode::VIEW_POINT_ORIENTED);
                                                                                                                                  auto *sp = Sprite::create("jacket/"+img);
                                                                                                                                  
                                                                                                                                  double rad = MATH_DEG_TO_RAD(theta);
                                                                                                                                  Vec3 v(480*sin(rad), 0.0, 4.8*cos(rad));
                                                                                                                                  PointWithDepth point;
                                                                                                                                  point.SetWorldPosition(v.x, v.y, v.z);
                                                                                                                                  CCLOG("%f, %f, %f\n", point.x, point.y, point.GetScale());
                                                                                                                                  
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
                                                                                                                              //jacketNode->setScale(0.7);
                                                                                                                              jacketNode->setPosition(480, 380);
                                                                                                                              this->addChild(jacketNode);
                                                                                                                            
                                                                                                                          }), NULL));
                                          
                                          CocosDenshion::SimpleAudioEngine::getInstance()->preloadEffect("Sound/SE/decide.mp3");
                                          CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("Sound/SE/decide.mp3");
                                          
                                      });
    
    
    return true;
}

void HomeScene::onEnterTransitionDidFinish()
{
    
    auto homeScene = this->getChildByName("HomeSceneLayer");
    auto action = cocostudio::timeline::ActionTimelineCache::getInstance()->createAction("res/HomeScene.csb");
    
    homeScene->runAction(action);
    action->gotoFrameAndPlay(0, false);
    CocosDenshion::SimpleAudioEngine::getInstance()->preloadBackgroundMusic("Sound/BGM/title_bgm.mp3");
    CocosDenshion::SimpleAudioEngine::getInstance()->playBackgroundMusic("Sound/BGM/title_bgm.mp3", true);

}