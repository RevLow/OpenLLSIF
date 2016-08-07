//
//  ConfigLayer.cpp
//  OpenLLSIF
//
//  Created by RevLow on 2015/08/01.
//
//

#include "ConfigLayer.h"
#include "ui/CocosGUI.h"
#include "cocostudio/CocoStudio.h"
#include <uuid/uuid.h>
#include "SimpleAudioEngine.h"
//#include "AudioManager.h"
#include "RhythmAdjustScene.h"
#import "external/unzip/unzip.h"


using namespace Config;

//作成するボタンの配列をあらかじめ作成しておく
//配列には使う画像へのファイルのパスを代入しておく
const int MAX_BUTTON_COUNT=2;

const std::string ButtonArray[MAX_BUTTON_COUNT] = {"res/Image/Config/Install_Button.png", "res/Image/Config/Adjust_Rhythm.png"};
const std::string ButtonOverArray[MAX_BUTTON_COUNT] = {"res/Image/Config/Install_Button_Click.png", "res/Image/Config/Adjust_Rhythm_click.png"};


//ボタンのクリックを制御する
inline void ButtonEnableDisable(ui::Button* button, bool isable)
{
    button->setTouchEnabled(isable);
    button->setEnabled(isable);
    button->setBright(isable);
}


/*=================================================================================================*/
/*                         メインのウィンドウの作成                                                    */
/*                                                                                                 */
/*=================================================================================================*/

/*
 メインウィンドウの初期化処理
 */
bool ConfigLayer::init()
{
    if(!Layer::init())
    {
        return false;
    }
    
    //背景画像を設定
    Sprite* background = Sprite::create("res/Image/Config/MainWindow_Background.png");
    addChild(background);
    
    //ウィンドウそのものののサイズを設定
    window_size = background->getContentSize();
    
    //ウインドウを閉じるボタンを作成
    cocos2d::ui::Button* exitButton = cocos2d::ui::Button::create("res/Image/Config/Exit_Button.png", "res/Image/Config/Exit_Button_Click.png");
    addChild(exitButton);
    exitButton->setPosition(Vec2(
                                 window_size.width/2 - (exitButton->getContentSize().width / 2),
                                 window_size.height/2 - (exitButton->getContentSize().height/2)));
    //ボタンを閉じる処理を設定
    exitButton->addClickEventListener([this](Ref* sender)
    {
        //close.mp3を鳴らす
        std::string filePath = "Sound/SE/close.mp3";
        std::string fullPath = FileUtils::getInstance()->fullPathForFilename(filePath);
        //AudioManager::getInstance()->play(fullPath, AudioManager::SE);
        CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(fullPath.c_str());
        this->runAction(Sequence::create(
                                         Spawn::create(ScaleTo::create(0.05f, 0.5f),
                                                       FadeTo::create(0.05f, 0), NULL),
                                         CallFunc::create([this]()
                                                        {
                                                            //このノードの親を取得し、親から見てこのノードを削除する
                                                            auto parentNode = this->getParent();
                                                            parentNode->removeChild(this);
                                                        })
                                         
                                         , NULL));
        

    });
    
    
    //テーブルビューを設定
    TableView* tableView = TableView::create(this, Size(window_size.width - 50, window_size.height - 50));
    tableView->setDirection(TableView::Direction::VERTICAL);
    
    tableView->setVerticalFillOrder(TableView::VerticalFillOrder::TOP_DOWN);
    tableView->setPosition(Vec2((-window_size.width / 2) + 20, (-window_size.height / 2)));
    tableView->setDelegate(this);
    addChild(tableView);
    tableView->reloadData();
    
    return true;
}

/*
 メインウィンドウのセルのサイズ指定
 幅は窓の幅、高さはボタンの大きさから170位を指定する
 */
Size ConfigLayer::cellSizeForTable(TableView* table)
{
    return Size(window_size.width, 170);
}

/*
 idx番目のCellが描画されるときに呼ばれるDelegateメソッド
 ひとつに最大３つのボタンを配置させる
 */
TableViewCell* ConfigLayer::tableCellAtIndex(TableView* table, ssize_t idx)
{
    int index = idx * 3;
    // セル
    TableViewCell *cell = table->dequeueCell();
    
    cell = new TableViewCell();
    cell->autorelease();
    
    float x = 0;
    float y = 76;
    
    for (int i=0; i < 3; i++)
    {
        if(index + i >= MAX_BUTTON_COUNT) break;
        ui::Button* btn = ui::Button::create(ButtonArray[index+i], ButtonOverArray[index+i]);
        x += (10+btn->getContentSize().width / 2);
        btn->setPosition(Vec2(x,y));
        btn->setTag(index + i);
        btn->addClickEventListener(CC_CALLBACK_1(ConfigLayer::ButtonClick,this));
        
        cell->addChild(btn);
        x += (10+btn->getContentSize().width / 2);
    }
    
    return cell;
}

// セル数
ssize_t ConfigLayer::numberOfCellsInTableView(TableView *table)
{
    return (int)((MAX_BUTTON_COUNT / 3)+1);
}

/*
 ボタンをクリックしたときに実行される
 senderから設定されたTagを取得し、取得したものから
 どの処理を行うかを決める
 */
void ConfigLayer::ButtonClick(Ref* sender)
{
    auto btn = (ui::Button*)sender;
    int tag = btn->getTag();
    
    //close.mp3を鳴らす
    std::string filePath = "Sound/SE/decide.mp3";
    std::string fullPath = FileUtils::getInstance()->fullPathForFilename(filePath);
    //AudioManager::getInstance()->play(fullPath, AudioManager::SE);
    CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(fullPath.c_str());
    switch (tag)
    {
        case 0:
        {
            //Installボタンを押した場合はInstallレイヤーを作成し、のせる
            auto installLayer = InstallLayer::create();
            this->addChild(installLayer);
            
            //イベントリスナーを追加する
            auto listener = EventListenerTouchOneByOne::create();
            listener->setSwallowTouches(true);
            listener->onTouchBegan = [](Touch* touch, Event* event){
                return true;
            };
            //重なりのPriorityにinstallLayerを利用する
            this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, installLayer);
            installLayer->setAnchorPoint(Vec2(0.0,0.0));
            break;
        }
        case 1:
        {
            CCLOG("ここでリズム調整シーンに移動させる");
            Scene* scene = RhythmAdjustScene::createScene();
            Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene, Color3B::BLACK));
            break;
        }
    }
}

#pragma mark - Install Layer Implements

bool InstallLayer::init()
{
    if(!Layer::init())
    {
        return false;
    }
    
    //背景画像を設定
    Sprite* background = Sprite::create("res/Image/Config/Song_Install_Window.png");
    addChild(background);
    
    window_size = background->getContentSize();
    //ウインドウを閉じるボタンを作成
    cocos2d::ui::Button* exitButton = cocos2d::ui::Button::create("res/Image/Config/Exit_Button.png", "res/Image/Config/Exit_Button_Click.png");
    addChild(exitButton);
    exitButton->setPosition(Vec2(
                                 window_size.width/2 - (exitButton->getContentSize().width / 2),
                                 window_size.height/2 - (exitButton->getContentSize().height/2)));
    
    //ボタンを閉じる処理を設定
    exitButton->addClickEventListener([this](Ref* sender)
                                      {
                                          //close.mp3を鳴らす
                                          std::string filePath = "Sound/SE/close.mp3";
                                          std::string fullPath = FileUtils::getInstance()->fullPathForFilename(filePath);
                                          //AudioManager::getInstance()->play(fullPath, AudioManager::SE);
                                          CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(fullPath.c_str());
                                          
                                          this->runAction(Sequence::create(
                                                                           Spawn::create(ScaleTo::create(0.05f, 0.5f),
                                                                                         FadeTo::create(0.05f, 0), NULL),
                                                                           CallFunc::create([this]()
                                                                                            {
                                                                                                //このノードの親を取得し、親から見てこのノードを削除する
                                                                                                auto parentNode = this->getParent();
                                                                                                parentNode->removeChild(this);
                                                                                            })
                                                                           
                                                                           , NULL));
                                          
                                          
                                      });
    
    //ファイルのリストを作成する
    std::string docPath = FileUtils::getInstance()->getWritablePath();
    //Documentsフォルダ内の要素を取得。ただし、zipファイルでフィルタリングを行う
    zipFileList = getContentsList(docPath, false, "zip");
    
    
    
    //テーブルビューを設定
    TableView* tableView = TableView::create(this, Size(window_size.width - 50, window_size.height - 50));
    tableView->setDirection(TableView::Direction::VERTICAL);
    tableView->setName("SongTable");
    
    tableView->setVerticalFillOrder(TableView::VerticalFillOrder::TOP_DOWN);
    tableView->setPosition(Vec2((-window_size.width / 2) + 20, (-window_size.height / 2) - 30));
    tableView->setDelegate(this);
    addChild(tableView);
    tableView->reloadData();
    
    
    return true;
}


//TableViewDataSourceの抽象メソッド
Size InstallLayer::cellSizeForTable(TableView* table)
{
    return Size(window_size.width, 100);
}


TableViewCell* InstallLayer::tableCellAtIndex(TableView* table,ssize_t idx)
{
    // セル
    TableViewCell *cell = table->dequeueCell();
    
    cell = new TableViewCell();
    cell->autorelease();
    
    // Background
 
    Sprite* bg = Sprite::create();
    bg->setAnchorPoint(Point(0, 0));
    bg->setTextureRect(Rect(0, 0, window_size.width, 100));
    bg->setColor(Color3B(245,245,245));
    cell->addChild(bg);

    
    std::string zipFile = zipFileList.at(idx);
    std::string docDir = FileUtils::getInstance()->getWritablePath();
    

    unsigned long size = 0;
    //plistの情報を取得
    unsigned char* plistBuff = FileUtils::getInstance()->getFileDataFromZip(docDir+"/"+zipFile, "fileInfo.plist", (ssize_t*)&size);
    
    //plistから楽曲の情報を取得
    ValueMap values = FileUtils::getInstance()->getValueMapFromData((const char*)plistBuff, size);
    auto imgFileName = values.at("Cover").asString();
    auto text = values.at("Name").asString();
    
    
    //不要になったplistのバッファを解放
    free((void*)plistBuff);
    
    unsigned char* imgBuff = FileUtils::getInstance()->getFileDataFromZip(docDir+"/"+zipFile,imgFileName, (ssize_t*)&size);
    
    auto img = new Image();
    //autoreleaseプールに追加しておく
    img->autorelease();
    img->initWithImageData(imgBuff, size);
    
    //不要になったimageのバッファを解放
    free(imgBuff);
    
    auto texture = new Texture2D();
    //autoreleaseプールに追加しておく
    texture->autorelease();
    texture->initWithImage(img);
    
    auto *sp = Sprite::createWithTexture(texture);
    sp->setScale(0.2f, 0.2f);
    sp->setPosition(Vec2(50, 50));
    cell->addChild(sp);
    
    
    // テキスト部分
    auto *label_2 = LabelTTF::create(text.c_str(), "Arial", 36);
    
    label_2->setAnchorPoint(Point(0, 0));
    label_2->setPosition(Point(100, 25));
    label_2->setColor(Color3B(64,64,64));
    cell->addChild(label_2);
    
    // ボーダーライン
    Sprite* line = Sprite::create();
    line->setAnchorPoint(Vec2(0, 0));
    line->setTextureRect(Rect(0, 0, window_size.width, 1));
    line->setColor(Color3B(124,124,124));
    cell->addChild(line);
    
    cell->setName(text);
    
    return cell;
}


ssize_t InstallLayer::numberOfCellsInTableView(TableView* table)
{
    return zipFileList.size();
}

//TableViewDelegateの抽象メソッド
void InstallLayer::tableCellTouched(TableView* table,TableViewCell* cell)
{
    installTargetFile = zipFileList.at(cell->getIdx());
    installTargetFileName = cell->getName();
    
    auto dialog_Window = CSLoader::getInstance()->createNode("res/Dialog_Window.csb");
    dialog_Window->setName("DialogLayer");
    dialog_Window->setAnchorPoint(Vec2(0.5,0.5));
    addChild(dialog_Window);
    
    //ボタンの設定
    auto cancel_Button = dialog_Window->getChildByName<ui::Button*>("No_Button");
    auto yes_Button = dialog_Window->getChildByName<ui::Button*>("Yes_Button");
    yes_Button->addClickEventListener(CC_CALLBACK_1(InstallLayer::installFile, this));
    cancel_Button->addClickEventListener([dialog_Window](Ref *sender){dialog_Window->removeFromParent();});

    //プログレスバーの設定
    auto installBar = dialog_Window->getChildByName<ui::LoadingBar*>("InstallBar");
    installBar->setPercent(0.0f);
    
    
    //メッセージの作成
    auto message_Text = dialog_Window->getChildByName<ui::Text*>("Message_Text");
    message_Text->setScale(0.8f);
    message_Text->setString(installTargetFileName+"のインストールを行います。\n完了後ファイルは自動的に削除されます。");
    
    
    //イベントリスナーを追加する
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = [](Touch* touch, Event* event){
        return true;
    };
    //重なりのPriorityにinstallLayerを利用する
    this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, dialog_Window);
}




//指定ファイルのインストールを実行する
void InstallLayer::installFile(Ref* sender)
{
    auto dialog_window = this->getChildByName("DialogLayer");
    auto yes_Button = dialog_window->getChildByName<ui::Button*>("Yes_Button");
    auto no_Button = dialog_window->getChildByName<ui::Button*>("No_Button");
    //yes_Buttonとno_Buttonをクリックできなくする
    ButtonEnableDisable(yes_Button, false);
    ButtonEnableDisable(no_Button, false);

    std::thread th1 = std::thread([&]()
                                  {
                                      //UUIDの生成
                                      uuid_t value;
                                      uuid_generate(value);
                                      
                                      uuid_string_t str;
                                      uuid_unparse(value, str);
                                      
                                      //この関数は標準ではなく、cocos2dxのコードを変更し、追加をしている
                                      const std::string cachePath = FileUtils::getInstance()->getCachedPath() +"Song/" + str + '/';
                                      const std::string writablePath(FileUtils::getInstance()->getWritablePath());
                                      
                                      ZipFile *zipFile = new ZipFile(writablePath+installTargetFile);
                                      FileUtils::getInstance()->createDirectory( cachePath );
                                      
                                      //cocos2dxを修正し、追加したzipファイル内のファイル数を取得する関数
                                      int delta = 100 / zipFile->size();
                                      
                                      for( std::string filename = zipFile->getFirstFilename(); !filename.empty(); filename = zipFile->getNextFilename() )
                                      {   
                                          if( *filename.rbegin() == '/' )
                                          {
                                              // It's a directory.
                                              cocos2d::FileUtils::getInstance()->createDirectory( cachePath + filename );
                                              
                                          }
                                          else
                                          {
                                              // It's a file.
                                              ssize_t filesize;
                                              unsigned char* filedata = zipFile->getFileData( filename, &filesize );
                                              {
                                                  const std::string fullPath( cachePath + filename );
                                                  FILE* file = fopen( fullPath.c_str(), "wb" );
                                                  fwrite( filedata, filesize, 1, file );
                                                  fclose( file );
                                              }
                                              free( filedata );
                                          }
                                          
                                          std::unique_lock<std::mutex> lock(mtx);
                                          //値をバーに代入するのが完了するまで待つ
                                          cv.wait(lock);
                                          
                                          if(progressValue + delta <= 100.0f) progressValue += delta;
                                          else progressValue = 100.0f;
                                      }
                                      delete zipFile;
                                      
                                      //インストールが完了したらファイルは自動削除する
                                      FileUtils::getInstance()->removeFile(writablePath+installTargetFile);
                                      if(progressValue <= 100.0f) progressValue = 100.0f;
                                      
                                      //最初から探索を行い、zipFileListからインストールしたファイルを削除する
                                      for(auto it = zipFileList.begin(); it != zipFileList.end();it++)
                                      {
                                          if(it->find(installTargetFile) != std::string::npos)
                                          {
                                              zipFileList.erase(it);
                                              break;
                                          }
                                      }
                                      //音楽の再生
                                      std::string fullpath = FileUtils::getInstance()->fullPathForFilename("Sound/SE/close.mp3");
                                      //AudioManager::getInstance()->play(fullpath,AudioManager::SE);
                                      CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(fullpath.c_str());
                                      
                                     //SongSelectionList.plistを修正
                                      fullpath = FileUtils::getInstance()->getCachedPath() + "SongSelectionList.plist";
                                      auto plistData = FileUtils::getInstance()->getValueVectorFromFile(fullpath);
                                      plistData.push_back(Value(cachePath+"fileInfo.plist"));
                                      FileUtils::getInstance()->writeValueVectorToFile(plistData, fullpath);
                                      
                                      Director::getInstance()->getScheduler()->performFunctionInCocosThread([this](){
                                          auto tableView = this->getChildByName<TableView*>("SongTable");
                                          tableView->reloadData();
                                      });
                                      
                                  }
    
    );
    
    
    this->schedule(schedule_selector(InstallLayer::loadingBarChange));
    th1.detach();
}

void InstallLayer::loadingBarChange(float deltaT)
{
    std::unique_lock<std::mutex> lock(mtx);
    auto dialog_window = this->getChildByName("DialogLayer");
    auto loadingBar = dialog_window->getChildByName<ui::LoadingBar*>("InstallBar");
    loadingBar->setPercent(progressValue);
 
    if(progressValue >= 100)
    {
        
        this->unschedule(schedule_selector(InstallLayer::loadingBarChange));
        dialog_window->removeFromParent();
    }
    
    //値の代入が完了したので次の処理に進ませるよう、状態の完了を伝える
    cv.notify_one();
}


