//
//  ConfigLayer.h
//  OpenLLSIF
//
//  Created by RevLow on 2015/08/01.
//
//

#ifndef __OpenLLSIF__ConfigLayer__
#define __OpenLLSIF__ConfigLayer__

#include <cocos2d.h>
#include <iostream>
#include <extensions/cocos-ext.h>

USING_NS_CC;
using namespace extension;


namespace Config
{
    //コンフィグウィンドウを作成するレイヤー
    class ConfigLayer : public Layer, public TableViewDataSource, public TableViewDelegate
    {
    public:
        CREATE_FUNC(ConfigLayer);
        virtual bool init();
        
        //TableViewDataSourceの抽象メソッド
        virtual Size cellSizeForTable(TableView* table);
        virtual TableViewCell* tableCellAtIndex(TableView* table,ssize_t idx);
        virtual ssize_t numberOfCellsInTableView(TableView* table);
        
        //TableViewDelegateの抽象メソッド
        virtual void tableCellTouched(TableView* table,TableViewCell* cell){};
        
        //TableViewDelegateが継承しているScrollViewの抽象メソッド
        virtual void scrollViewDidScroll(ScrollView* view){};
        virtual void scrollViewDidZoom(ScrollView* view){};
    protected:
        void ButtonClick(Ref* sender);
    private:
        Size window_size;
    };
    
    //コンフィグウィンドウから表示されるレイヤー
    class InstallLayer : public Layer
    {
    public:
        CREATE_FUNC(InstallLayer);
        virtual bool init();
        
    private:
        Size window_size;
        
//        //TableViewDataSourceの抽象メソッド
//        virtual Size cellSizeForTable(TableView* table);
//        virtual TableViewCell* tableCellAtIndex(TableView* table,ssize_t idx);
//        virtual ssize_t numberOfCellsInTableView(TableView* table);
//        
//        //TableViewDelegateの抽象メソッド
//        virtual void tableCellTouched(TableView* table,TableViewCell* cell);
//        
//        //TableViewDelegateが継承しているScrollViewの抽象メソッド
//        virtual void scrollViewDidScroll(ScrollView* view){};
//        virtual void scrollViewDidZoom(ScrollView* view){};
    };
}


#endif /* defined(__OpenLLSIF__ConfigLayer__) */
