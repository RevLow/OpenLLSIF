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
//#include "SimpleAudioEngine.h"
#include "AudioManager.h"
#include "PointWithDepth.h"
#include "PlayScene.h"
#include "ConfigLayer.h"


USING_NS_CC;

Scene* HomeScene::createScene(ViewScene entryScene)
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();
    
    // 'layer' is an autorelease object
    auto layer = HomeScene::create(entryScene);
    
    // add layer as a child to scene
    scene->addChild(layer);
    
    // return the scene
    return scene;
}

bool HomeScene::init(ViewScene entryScene)
{
    _currentScene = entryScene;
    touchable_index = UserDefault::getInstance()->getIntegerForKey("SONG_SELECTION", 0);
    
    if (!Layer::init()) {
        return false;
    }
    
    
    //ボタンのレイヤーを作成
    auto homeScene = CSLoader::getInstance()->createNode("res/HomeScene.csb");
    homeScene->setName("HomeSceneLayer");
    homeScene->setLocalZOrder(1);
    auto action = cocostudio::timeline::ActionTimelineCache::getInstance()->createAction("res/HomeScene.csb");
    
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
    
    auto configButton = homeScene->getChildByName<ui::Button*>("Config_Button");
    configButton->addClickEventListener([this](Ref* sender)
    {
        //decide.mp3を鳴らす
        std::string filePath = "Sound/SE/decide.mp3";
        std::string fullPath = FileUtils::getInstance()->fullPathForFilename(filePath);
        AudioManager::getInstance()->play(fullPath, AudioManager::SE);

        //getTouchDispatcher()->addTargetedDelegate(this, kCCMenuHandlerPriority - 1, true);

        auto size = Director::getInstance()->getVisibleSize();
        auto configLayer = Config::ConfigLayer::create();
        this->addChild(configLayer);
        
        //イベントリスナーを追加する
        auto listener = EventListenerTouchOneByOne::create();
        
        listener->setSwallowTouches(true);
        listener->onTouchBegan = [](Touch* touch, Event* event){
            return true;
        };
        //重なりのPriorityにConfigを利用する
        this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, configLayer);
        configLayer->setAnchorPoint(Vec2(0.0,0.0));
        configLayer->setPosition(Vec2(size.width / 2, size.height / 2));
        configLayer->setLocalZOrder(2);
        configLayer->setScale(0.5f);
        configLayer->runAction(ScaleTo::create(0.1f, 1.0f));
    });
    
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
    
    switch (_currentScene)
    {
        case ViewScene::Home:
            loadHomeScene();
            break;
        case ViewScene::Live:
            loadLiveScene();
            break;
        default:
            break;
    }

}

/*
 ホームボタンを押したときの処理
 背景をホームの背景に変更を行い、アニメーションを再生する
 */
void HomeScene::homeButton_action(Ref *ref)
{
    if(_currentScene == ViewScene::Home)
    {
        //今がホームシーンの場合は実行を行わない
        //decide.mp3を鳴らす
        std::string filePath = "Sound/SE/SE_005.mp3";
        std::string fullPath = FileUtils::getInstance()->fullPathForFilename(filePath);
        AudioManager::getInstance()->play(fullPath, AudioManager::SE);
        
        return;
    }
    
    //decide.mp3を鳴らす
    std::string filePath = "Sound/SE/decide.mp3";
    std::string fullPath = FileUtils::getInstance()->fullPathForFilename(filePath);
    AudioManager::getInstance()->play(fullPath, AudioManager::SE);
    
    
    //ボタンを押したときの動作を設定
    auto background = this->getChildByName("backgroundLayer");


    background->runAction(Sequence::create(FadeTo::create(0.3f, 0),CallFunc::create(CC_CALLBACK_0(HomeScene::loadHomeScene, this)), NULL));
    _currentScene = ViewScene::Home;
}
/*
 ホームシーンを生成
 */
void HomeScene::loadHomeScene()
{
    this->removeChildByName("backgroundLayer");
    this->removeChildByName("jacketLayer");
    auto newLayer = CSLoader::getInstance()->createNode("res/home_background.csb");
    auto action = cocostudio::timeline::ActionTimelineCache::getInstance()->createAction("res/home_background.csb");
    newLayer->setName("backgroundLayer");
    
    newLayer->runAction(action);
    action->gotoFrameAndPlay(0, true);
    this->addChild(newLayer, 0);
    if(AudioManager::getInstance()->isPlaying()) AudioManager::getInstance()->stop(AudioManager::BGM);
    std::string filePath = "Sound/BGM/title_bgm.mp3";
    std::string fullPath = FileUtils::getInstance()->fullPathForFilename(filePath);
    AudioManager::getInstance()->play(fullPath, AudioManager::BGM, true);
    //CocosDenshion::SimpleAudioEngine::getInstance()->playBackgroundMusic("Sound/BGM/title_bgm.mp3", true);
}


/*
ライブボタンを押したときの処理
背景をliveの背景に変更を行い、擬似的な3次元空間上にアルバムのジャケットを追加する
また、ボタンにはアクションを追加し、アクションはまた別の関数で定義を行っている
 */
void HomeScene::liveButton_action(Ref *ref)
{
    
    if(_currentScene == ViewScene::Live)
    {
        //今がホームシーンの場合は実行を行わない
        //decide.mp3を鳴らす
        std::string filePath = "Sound/SE/SE_005.mp3";
        std::string fullPath = FileUtils::getInstance()->fullPathForFilename(filePath);
        AudioManager::getInstance()->play(fullPath, AudioManager::SE);
        
        return;
    }

    //ボタンを押したときの動作を設定
    auto background = this->getChildByName("backgroundLayer");
    
    //decide.mp3を鳴らす
    std::string filePath = "Sound/SE/decide.mp3";
    std::string fullPath = FileUtils::getInstance()->fullPathForFilename(filePath);
    AudioManager::getInstance()->play(fullPath, AudioManager::SE);
    
    background->runAction(Sequence::create(FadeTo::create(0.3f, 0),CallFunc::create(CC_CALLBACK_0(HomeScene::loadLiveScene, this)), NULL));
    

    
    _currentScene = ViewScene::Live;
}
/*
 liveシーンを生成する
 */
void HomeScene::loadLiveScene()
{
    this->removeChildByName("backgroundLayer");
  
    auto newLayer = CSLoader::getInstance()->createNode("res/song_selection.csb");
    auto action = cocostudio::timeline::ActionTimelineCache::getInstance()->createAction("res/song_selection.csb");
    newLayer->setName("backgroundLayer");
    
    newLayer->runAction(action);
    action->gotoFrameAndPlay(0,true);
    this->addChild(newLayer, 0);
    
    //背景画像の作成
    Sprite* backgroundSprite = Sprite::create("background/background_expert.png");
    backgroundSprite->setName("backgroundImage");
    backgroundSprite->setPosition(480, 320);
    backgroundSprite->setLocalZOrder(-2);
    this->addChild(backgroundSprite);
    
    
    //ボタンの設定
    auto nextButton = newLayer->getChildByName<ui::Button*>("next_button");
    nextButton->addClickEventListener(CC_CALLBACK_1(HomeScene::nextAlbum_click, this));
    
    auto previousButton = newLayer->getChildByName<ui::Button*>("previous_button");
    previousButton->addClickEventListener(CC_CALLBACK_1(HomeScene::previousAlbum_click, this));
    
    std::string docDir = FileUtils::getInstance()->getCachedPath() + "Song/";
    //インストールディレクトリからフォルダのリストを取得
    auto fileList = getContentsList(docDir, true);
    
    //ファイルが存在している場合のみ
    if(fileList.size() > 0)
    {
        //CocosDenshion::SimpleAudioEngine::getInstance()->stopBackgroundMusic();
        AudioManager::getInstance()->stop(AudioManager::BGM);
        //ジャケットを配置するノードの作成
        //ジャケットサイズは500x500
        Node* jacketNode = Node::create();
        jacketNode->setName("jacketLayer");
        
        
        
        float theta = 0;
        float dTheta = 360 / fileList.size();
        float z = 0;
        
        int index = touchable_index;
        
        for(int i=0; i < fileList.size();i++)
        {
            auto songDir = fileList[index];
            
            auto plistData = FileUtils::getInstance()->getValueMapFromFile(docDir + songDir +"/fileInfo.plist");
            std::string jacketCover = plistData.at("Cover").asString();
            
            
            auto *sp = Sprite::create(docDir + songDir + '/' +jacketCover);
            //あらかじめタグ付けを行っておく事で回転のボタンを押したときの処理を行えるようにする
            sp->setTag(index);
            
            //コンテナに登録を行う
            //JacketInfoMap.push_back(songDir);
            JacketInfoMap[index] = songDir;
            
            if(i==0)
            {
                std::string bgm = plistData.at("BGM").asString();
                
                std::string fullPath = FileUtils::getInstance()->fullPathForFilename((docDir + songDir + '/' + bgm).c_str());

                AudioManager::getInstance()->play(fullPath, AudioManager::BGM, true);
            }
            
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
            
            index++;
            if(index >= fileList.size()) index = 0;
        }
        
        
        jacketNode->setLocalZOrder(-1);
        jacketNode->setScale(0.5f);
        jacketNode->setPosition(480, 380);
        this->addChild(jacketNode);
        
        
        //jacketNodeにイベントリスナーを追加する
        auto listener = EventListenerTouchOneByOne::create();
        
        //listener->setSwallowTouches(true);
        listener->onTouchBegan = CC_CALLBACK_2(HomeScene::jacket_touch, this);
        
        //重なりのPriorityにjacketNodeを利用する
        //これにより、jacketNodeより上にきたNodeのイベントが優先される(主にConfigのレイヤーのため)
        this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, jacketNode);
    }
}


/*
 live画面中での次のボタンを押したときのジャケットに対する処理
 addChildするときに指定したタグをもとに円形に配置したとなりのジャケットを参照、そして移動、スケールを繰り返す
 */
void HomeScene::nextAlbum_click(Ref *ref)
{
    ui::Button* nstButton = (ui::Button*)ref;
    nstButton->setEnabled(false);
    
    //CocosDenshion::SimpleAudioEngine::getInstance()->preloadEffect("Sound/SE/selection.mp3");
    //CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("Sound/SE/selection.mp3");
    //selection.mp3を鳴らす
    std::string filePath = "Sound/SE/selection.mp3";
    std::string fullPath = FileUtils::getInstance()->fullPathForFilename(filePath);
    AudioManager::getInstance()->play(fullPath, AudioManager::SE);
    
    auto jacketNode = this->getChildByName("jacketLayer");
    
    //もしも、jacketNodeが存在しない場合
    //-> ひとつも楽曲が無い場合、ボタンを有効化し戻る
    if(jacketNode == NULL)
    {
        nstButton->setEnabled(true);
        return;
    }
    
    //クリック可能なスプライトを変更する
    touchable_index = touchable_index == 0 ? jacketNode->getChildrenCount() - 1 : touchable_index-1;
    
    
    //ゲームで使うファイルを取得する
    std::string zipFileName = JacketInfoMap.at(touchable_index);
    std::string docPath = FileUtils::getInstance()->getCachedPath() + "Song/" + zipFileName;
    
    auto plistData = FileUtils::getInstance()->getValueMapFromFile(docPath+"/fileInfo.plist");
    std::string songFileName = docPath + "/" + plistData.at("BGM").asString();
    AudioManager::getInstance()->play(songFileName, AudioManager::BGM);
    
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
                                          CallFunc::create([finalNode, zeroLocalZ, nstButton]
                                                           {
                                                               nstButton->setEnabled(true);
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
    ui::Button* prevButton = (ui::Button*)ref;
    prevButton->setEnabled(false);
    
    //CocosDenshion::SimpleAudioEngine::getInstance()->preloadEffect("Sound/SE/selection.mp3");
    //CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("Sound/SE/selection.mp3");
    //selection.mp3を鳴らす
    std::string filePath = "Sound/SE/selection.mp3";
    std::string fullPath = FileUtils::getInstance()->fullPathForFilename(filePath);
    AudioManager::getInstance()->play(fullPath, AudioManager::SE);
    auto jacketNode = this->getChildByName("jacketLayer");
    
    //もしも、jacketNodeが存在しない場合
    //-> ひとつも楽曲が無い場合、ボタンを有効化し、戻る
    if(jacketNode == NULL)
    {
        prevButton->setEnabled(true);
        return;
    }
    
    //クリック可能なスプライトを変更する
    touchable_index = touchable_index == jacketNode->getChildrenCount() - 1 ? 0 : touchable_index+1;
    
    //ゲームで使うファイルを取得する
    std::string zipFileName = JacketInfoMap.at(touchable_index);
    std::string docPath = FileUtils::getInstance()->getCachedPath() + "Song/" + zipFileName;
    
    auto plistData = FileUtils::getInstance()->getValueMapFromFile(docPath+"/fileInfo.plist");
    auto songFileName = docPath + "/" + plistData.at("BGM").asString();
    AudioManager::getInstance()->play(songFileName, AudioManager::BGM);
    
    
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
                                          CallFunc::create([finalNode, zeroLocalZ, prevButton]
    {
        finalNode->setLocalZOrder(zeroLocalZ);
        prevButton->setEnabled(true);
        
    
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
        //現在のtouchable_indexをセーブ
        UserDefault::getInstance()->setIntegerForKey("SONG_SELECTION", touchable_index);
        
        //決定音を鳴らす
        std::string filePath = "Sound/SE/decide2.mp3";
        std::string fullPath = FileUtils::getInstance()->fullPathForFilename(filePath);
        AudioManager::getInstance()->play(fullPath, AudioManager::SE);
        
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
                                          CallFunc::create([node,referenceSprite, this]()
                                                            {
                                                                //アニメーション終了後にreferenceSpriteを消去
                                                                node->removeChild(referenceSprite);
                                                                //ここで次のシーンへの遷移を行う
                                                                //CocosDenshion::SimpleAudioEngine::getInstance()->stopBackgroundMusic();
                                                                AudioManager::getInstance()->stop(AudioManager::BGM);
                                                                //ゲームで使うファイルを取得する
                                                                std::string zipFileName = JacketInfoMap.at(touchable_index);
                                                                std::string docPath = FileUtils::getInstance()->getCachedPath() + "Song/" + zipFileName;
                                                                //zipファイルの情報をもとにシーンを作成する
                                                                Scene* scene = PlayScene::createScene(docPath, GameLevel::EXPERT);
                                                                Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene, Color3B::BLACK));
                                                            }), NULL);
        referenceSprite->runAction(seqAction);
        
        
        return true;
    }
    
    //touchPointが範囲内に入っていれば
    return false;
}