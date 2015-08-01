//
//  ConfigLayer.cpp
//  OpenLLSIF
//
//  Created by RevLow on 2015/08/01.
//
//

#include "ConfigLayer.h"
#include "ui/CocosGUI.h"

using namespace Config;

//作成するボタンの配列をあらかじめ作成しておく
//配列には使う画像へのファイルのパスを代入しておく
const int MAX_BUTTON_COUNT=1;

const std::string ButtonArray[MAX_BUTTON_COUNT] = {"res/Image/Config/Install_Button.png"};
const std::string ButtonOverArray[MAX_BUTTON_COUNT] = {"res/Image/Config/Install_Button_Click.png"};


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
    
    switch (tag)
    {
        case 0:
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
            //installLayer->setPosition(window_size.width / 2, window_size.height / 2);
            break;
    }
}







/*=================================================================================================*/
/*                         Installのウィンドウの作成                                                  */
/*                                                                                                 */
/*=================================================================================================*/


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
    
    return true;
}