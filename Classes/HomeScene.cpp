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


/*!
 * パスからファイル名を取り除いたパスを抽出
 * @param[in] path パス
 * @return フォルダパス
 */
inline std::string GetFolderPath(const std::string &path)
{
    size_t pos1;
    
    pos1 = path.rfind('\\');
    if(pos1 != std::string::npos){
        return path.substr(0, pos1+1);
        
    }
    
    pos1 = path.rfind('/');
    if(pos1 != std::string::npos){
        return path.substr(0, pos1+1);
    }
    
    return "";
}
/*!
 * パスからファイルの親フォルダ名を取り出す
 * @param[in] path ファイルパス
 * @return 親フォルダ名
 */
inline std::string GetParentFolderName(const std::string &path)
{
	std::string::size_type pos1, pos0;
	pos1 = path.find_last_of("\\/");
	pos0 = path.find_last_of("\\/", pos1-1);
    
	if(pos0 != std::string::npos && pos1 != std::string::npos){
		return path.substr(pos0+1, pos1-pos0-1);
	}
	else{
		return "";
	}
}

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
    auto backgroundLayer = this->getChildByName("backgroundLayer");
    if(backgroundLayer != nullptr) this->removeChild(backgroundLayer);
    auto jacketLayer = this->getChildByName("jacketLayer");
    if (jacketLayer != nullptr) {
        this->removeChild(jacketLayer);
    }
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
    auto fullpath = FileUtils::getInstance()->getCachedPath() + "SongSelectionList.plist";
    CCLOG(fullpath.c_str());
    auto songList = FileUtils::getInstance()->getValueVectorFromFile(fullpath);
   
    int songListSize = songList.size();
    
    if (songListSize > 0)
    {
        //もし、保持しているスタックに何も入っていないなら
        if(songStack.size() == 0)
        {
            for (int i=0;i<songList.size();i++)
            {
                songStack.push_back(songList[i].asString());
            }
        }
        
        auto jacketAttributes = FileUtils::getInstance()->getValueVectorFromFile("UiSongList.plist");
        
        Node* jacketNode = Node::create();
        jacketNode->setName("jacketLayer");
        
        for (int i = 0; i < jacketAttributes.size(); ++i)
        {
            int index = 0;
            if(i % 2 == 0) index = i/2;
            else index = songList.size() - ((i/2) + 1);
            
            Vec2 position(jacketAttributes[i].asValueMap()["PositionX"].asFloat(),
                          jacketAttributes[i].asValueMap()["PositionY"].asFloat());
            float scale = jacketAttributes[i].asValueMap()["Scale"].asFloat();
            int zOdr = jacketAttributes[i].asValueMap()["Z"].asInt();
            
            if (i >= songList.size())
            {
                break;
            }
            
            std::string filePath = songStack[index];
            auto plist = FileUtils::getInstance()->getValueMapFromFile(filePath);
            std::string fullPath = GetFolderPath(songStack[index]) + plist["Cover"].asString();
            Sprite *sp = Sprite::create(fullPath);
            if(sp->getContentSize().width != 500)
            {
                sp = Sprite::create(fullPath, 500.0 / sp->getContentSize().width);
            }

            int halfSize = songStack.size() / 2;
            if (halfSize >= 2) {
                halfSize = 2;
            }
            int tag = index + halfSize;
            if(tag >= songStack.size()) tag -= songStack.size();
            CCLOG("%d, %d", index, tag);
            sp->setTag(tag);
            sp->setPosition(position);
            sp->setScale(scale);
            sp->setZOrder(zOdr);
            
            jacketNode->addChild(sp);
            
            if (i == 0)
            {
                AudioManager::getInstance()->stop(AudioManager::BGM);
                fullPath = GetFolderPath(songStack[index]) + plist["BGM"].asString();
                AudioManager::getInstance()->play(fullPath, AudioManager::BGM);
            }
        }
        
        addChild(jacketNode);
        
        //jacketNodeにイベントリスナーを追加する
        auto listener = EventListenerTouchOneByOne::create();

        //listener->setSwallowTouches(true);
        listener->onTouchBegan = CC_CALLBACK_2(HomeScene::jacket_touch, this);

        //重なりのPriorityにjacketNodeを利用する
        //これにより、jacketNodeより上にきたNodeのイベントが優先される(主にConfigのレイヤーのため)
        this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, jacketNode);
        
        
        newLayer->getChildByName<ui::Button*>("next_button")->addClickEventListener(CC_CALLBACK_1(HomeScene::nextAlbum_click, this));
        newLayer->getChildByName<ui::Button*>("previous_button")->addClickEventListener(CC_CALLBACK_1(HomeScene::previousAlbum_click, this));
    }
}


/*
 live画面中での次のボタンを押したときのジャケットに対する処理
 addChildするときに指定したタグをもとに円形に配置したとなりのジャケットを参照、そして移動、スケールを繰り返す
 */
void HomeScene::nextAlbum_click(Ref *ref)
{
    ui::Button* nstButton = dynamic_cast<ui::Button*>(ref);
    nstButton->setEnabled(false);
    

    //selection.mp3を鳴らす
    std::string filePath = "Sound/SE/selection.mp3";
    std::string fullPath = FileUtils::getInstance()->fullPathForFilename(filePath);
    AudioManager::getInstance()->play(fullPath, AudioManager::SE);
    
    Node* jacketNode = getChildByName("jacketLayer");
    

    //もしも、jacketNodeが存在しない場合
    //-> ひとつも楽曲が無い場合、ボタンを有効化し戻る
    if(jacketNode == NULL)
    {
        nstButton->setEnabled(true);
        return;
    }

    Sprite *node = jacketNode->getChildByTag<Sprite*>(0);
    
    Vec2 position(node->getPositionX(), node->getPositionY());
    float scale = node->getScale();
    int zOdr = node->getLocalZOrder();
    int tag = node->getTag();
    node->setLocalZOrder(zOdr - 1);
    node->runAction(Sequence::create(Spawn::create(MoveTo::create(0.2f, Vec2(Director::getInstance()->getVisibleSize().width / 2, Director::getInstance()->getVisibleSize().height/ 2)),
                                                   ScaleTo::create(0.2f, scale*0.25f), NULL),
                                   CallFunc::create([node, jacketNode]()
                                                    {
                                                        jacketNode->removeChild(node);
                                                    })
                                   , NULL));
    
    for(int i = 1;i < 5;i++)
    {
        Sprite *currentSprite = jacketNode->getChildByTag<Sprite*>(i);
        if (currentSprite == nullptr)
        {
            break;
        }
        currentSprite->runAction(Sequence::create(Spawn::create(MoveTo::create(0.2f, position), ScaleTo::create(0.2f, scale), NULL),
                                         CallFunc::create([currentSprite, zOdr, tag]()
                                                          {
                                                              currentSprite->setLocalZOrder(zOdr);
                                                              currentSprite->setTag(tag);
                                                          })
                                                  , NULL));
        position = currentSprite->getPosition();
        scale = currentSprite->getScale();
        zOdr = currentSprite->getLocalZOrder();
        tag = currentSprite->getTag();
    }
    
    //スプライトを追加するindex番号を取得
    int index;
    switch (songStack.size())
    {
        case 0:
            index = 0;
            break;
        case 1:
            index = 0;
            break;
        case 2:
            index = 1;
            break;
        case 3:
            index = 2;
            break;
        case 4:
            index = 2;
            break;
        default:
            index = 3;
            break;
    }
    
    //空領域に追加
    std::string songPlist = songStack[index];
    auto plist = FileUtils::getInstance()->getValueMapFromFile(songPlist);
    Sprite *sp = Sprite::create(GetFolderPath(songPlist) + plist["Cover"].asString());
    if(sp->getContentSize().width != 500)
    {
        sp = Sprite::create(GetFolderPath(songPlist) + plist["Cover"].asString(), 500.0 / sp->getContentSize().width);
    }
    
    sp->setPosition(Director::getInstance()->getVisibleSize().width / 2,
                    Director::getInstance()->getVisibleSize().height / 2);
    sp->setScale(scale * 0.25f);
    sp->setLocalZOrder(zOdr - 1);
    sp->setTag(tag);
    sp->runAction(Sequence::create(Spawn::create(MoveTo::create(0.2f, position), ScaleTo::create(0.2f, scale), NULL),
                                   CallFunc::create([sp, zOdr, nstButton]()
                                                    {
                                                        sp->setLocalZOrder(zOdr);
                                                        nstButton->setEnabled(true);
                                                    })
                                   , NULL));
    jacketNode->addChild(sp);
    
    //スタックをずらす
    songPlist = songStack[0];
    for (int x = 0; x < songStack.size() - 1; x++)
    {
        songStack[x] = songStack[x+1];
    }
    songStack[songStack.size()-1] = songPlist;

    //スタック変更後の先頭曲を鳴らす
    plist = FileUtils::getInstance()->getValueMapFromFile(songStack[0]);
    fullPath = GetFolderPath(songStack[0]) + plist["BGM"].asString();
    AudioManager::getInstance()->play(fullPath, AudioManager::BGM);
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
    
    int nodeIndex = 4;
    Sprite* node =nullptr;
    while (node == nullptr) {
        node = jacketNode->getChildByTag<Sprite*>(nodeIndex);
        nodeIndex--;
    }
    
    
    Vec2 position(node->getPositionX(), node->getPositionY());
    float scale = node->getScale();
    int zOdr = node->getLocalZOrder();
    int tag = node->getTag();
    node->setLocalZOrder(zOdr - 1);
    node->runAction(Sequence::create(Spawn::create(MoveTo::create(0.2f, Vec2(Director::getInstance()->getVisibleSize().width / 2, Director::getInstance()->getVisibleSize().height/ 2)),
                                                   ScaleTo::create(0.2f, scale*0.25f), NULL),
                                     CallFunc::create([node, jacketNode]()
                                                      {
                                                          jacketNode->removeChild(node);
                                                      })
                                     , NULL));
    
    
    for(int i = nodeIndex;i >= 0;--i)
    {
        Sprite *currentSprite = jacketNode->getChildByTag<Sprite*>(i);
        if (currentSprite == nullptr)
        {
            break;
        }
        currentSprite->runAction(Sequence::create(Spawn::create(MoveTo::create(0.2f, position), ScaleTo::create(0.2f, scale), NULL),
                                                  CallFunc::create([currentSprite, zOdr, tag]()
                                                                   {
                                                                       currentSprite->setLocalZOrder(zOdr);
                                                                       currentSprite->setTag(tag);
                                                                   })
                                                  , NULL));
        position = currentSprite->getPosition();
        scale = currentSprite->getScale();
        zOdr = currentSprite->getLocalZOrder();
        tag = currentSprite->getTag();
    }

    int index;
    switch (songStack.size())
    {
        case 0:
            index = 0;
            break;
        case 1:
            index = -1;
            break;
        case 2:
            index = -2;
            break;
        case 3:
            index = -2;
            break;
        case 4:
            index = -3;
            break;
        default:
            index = -3;
            break;
    }
    
    index += songStack.size();
    
    //空領域に追加
    std::string songPlist = songStack[index];
    auto plist = FileUtils::getInstance()->getValueMapFromFile(songPlist);
    Sprite *sp = Sprite::create(GetFolderPath(songPlist) + plist["Cover"].asString());
    if(sp->getContentSize().width != 500)
    {
        sp = Sprite::create(GetFolderPath(songPlist) + plist["Cover"].asString(), 500.0 / sp->getContentSize().width);
    }
    
    sp->setPosition(Director::getInstance()->getVisibleSize().width / 2,
                    Director::getInstance()->getVisibleSize().height / 2);
    sp->setScale(scale * 0.25f);
    sp->setLocalZOrder(zOdr - 1);
    sp->setTag(tag);
    sp->runAction(Sequence::create(Spawn::create(MoveTo::create(0.2f, position), ScaleTo::create(0.2f, scale), NULL),
                                   CallFunc::create([sp, zOdr, prevButton]()
                                                    {
                                                        sp->setLocalZOrder(zOdr);
                                                        prevButton->setEnabled(true);
                                                    })
                                   , NULL));
    jacketNode->addChild(sp);
    
    //スタックをずらす
    songPlist = songStack[songStack.size() - 1];
    for (int x = songStack.size()-1; x > 0; x--)
    {
        songStack[x] = songStack[x-1];
    }
    songStack[0] = songPlist;
    
    //スタック変更後の先頭曲を鳴らす
    plist = FileUtils::getInstance()->getValueMapFromFile(songStack[0]);
    fullPath = GetFolderPath(songStack[0]) + plist["BGM"].asString();
    AudioManager::getInstance()->play(fullPath, AudioManager::BGM);
}

/*
 ジャケットをクリックしたときの処理を行う
 songStackの先頭を次のシーンに渡す
 参考: http://ladywendy.com/lab/cocos2d-x-v3/170.html
 */
bool HomeScene::jacket_touch(cocos2d::Touch* touch, cocos2d::Event* e)
{
    Node* node = this->getChildByName("jacketLayer");
    
    //クリックされたターゲットを取得する
    //auto target = (Sprite*)e->getCurrentTarget();
    
    
    //クリックされたスプライトの領域
    int halfSize = songStack.size() / 2;
    if (halfSize >= 2) {
        halfSize = 2;
    }
    auto referenceSprite = (Sprite*)node->getChildByTag(halfSize);
    
    Rect targetBox = referenceSprite->getBoundingBox();
    
    //クリックされた位置を取得
    Vec2 touchPoint = node->convertTouchToNodeSpace(touch);
    
    if(targetBox.containsPoint(touchPoint))
    {
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
        
        
        //SongStackを書き出す
        ValueVector plistVector;
        for (std::string str : songStack)
        {
            Value v(str);
            plistVector.push_back(v);
        }
        fullPath = FileUtils::getInstance()->getCachedPath() + "SongSelectionList.plist";
        FileUtils::getInstance()->writeValueVectorToFile(plistVector, fullPath);
        
        //選択項目をアニメーション
        auto seqAction = Sequence::create(Spawn::create(
                                                        ScaleTo::create(0.5f, 1.0),
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
                                                                std::string zipFileName = GetParentFolderName(songStack[0]);
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