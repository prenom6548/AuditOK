/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef CORE_H
#define CORE_H


#include <QObject>
#include <QString>
#include <QTimer>
#include <QGSettings>
#include <QStandardItemModel>
#include <QNetworkAccessManager>
#include <QJsonParseError>
#include <QJsonObject>
#include <QEventLoop>
#include <QNetworkReply>
#include <QAbstractItemModel>
#include <QThread>
#include <QVariant>
#include <QModelIndexList>
#include <QFont>
#include <QFontMetrics>
#include "model/dbus.h"
#include "model/network.h"
#include "model/dataparser.h"
#include "controller/core/errorreact.h"
#include "model/iplocation.h"
#include "model/datarequest.h"
#include "model/searchmarch.h"
#include <QPersistentModelIndex>
#include <buried_point.hpp>
#include "model/titlesearchmodel.h"
class Core : public QObject
{
    Q_OBJECT
    friend class TestCore;

signals:
    void coreProgremExit();                            //结束进程
    void showCenter();                                 //显示在屏幕中间
    void showLifeStyle(LifeStyle m_lifeStyle);         //发送信号，显示生活指数信息
    void createTrayIcon(QString code);                 //创建托盘
    void showCurrent(CurrentWeather m_currentWeather); //发送信号，显示当前城市信息
    void showForcast(ForecastWeather forecastWeather); //发送信号，显示城市预测信息
    void showHourly(QVariant var);                     //发送信号，显示城市逐小时信息
    void startView();                                  //开启前端
    void collectNull();                                //收藏城市为空--删光了
    void collectShow();                                //配置文件中收藏城市不为空
    void searchUiShow(bool hasCity);                   //搜索有结果或搜索输入为空
    void sigTitleSearchUiShow(bool hasCity);           //标题栏搜索有结果或搜索输入为空
    void searchNull();                                 //搜索为空
    void updateFinish();                               //收藏城市最后一个加载完成
    void sigShowBottomList(int itemSize);              // 显示主界面底部的收藏城市列表
    void sigDeleCollectCity();                         // 更换界面收藏图标样式
    void sigAddCollectCity();                          // 更换界面收藏图标样式
    void sigBottomModeGet();                           // 底部model加载完成

public:
    Core();
    QString initDbus(const QStringList &arguments); //初始化Dbus模块
    void changeCity(QString cityId);                //切换城市
    void changeCityFromSearch(QModelIndex index);   //切换到目标城市
    void addCollectCity(QModelIndex index);         //添加收藏城市
    void addBottomCollectCity(QModelIndex index);   //添加主界面最底部收藏城市
    void addCollectCityFromWidget(QString cityId);  //添加主界面收藏城市
    void collectCity(QString Id, QString name);     // 收藏城市
    void changeCityFromCollect(QModelIndex index);  //切换到目标城市
    void delCollectCity(QModelIndex index);         //添加收藏城市
    void delBottomCollectCity(QString cityId);      //删除主界面最底部收藏城市
    void delCollectCityFromWidget(QString cityId);  //主界面点击取消收藏
    void searchCity(QString inputText);             //搜索本地词库表
    void initCore(const QStringList &args = QStringList()); //初始化核心
    void loadCollectCity();                         //加载收藏城市
    void showInCenter();                            //显示在屏幕中间
    QStandardItemModel *getCityModel();             //获取搜索列表model指针
    QStandardItemModel *getCityTitleModel();        //获取标题栏搜索列表model指针
    QStandardItemModel *getCollectModel();          //获取收藏城市model指针
    QStandardItemModel *getBottomCollectModel();    //获取主界面最底部收藏城市model指针
    QString showCityWeather(const QString &cityName); // DBus接口调用的切换城市并显示主界面
    void allowLocationYes();
    void allowLocationNo();
    void progremExit(); //结束进程
    void refreshCity(); //刷新城市
    void showNAData(); //显示N/A数据

private:
    Dbus *m_dbus = nullptr;               // DBus模块对象
    Network *m_network = nullptr;         //网络模块
    DataParser *m_dataParser = nullptr;   //数据解析模块
    DataRequest *m_dataRequest = nullptr; //数据请求模块
    Iplocation *m_ipLocation = nullptr;   //初始化ip定位模块
    SearchMarch *m_searchMarch = nullptr; //初始化搜索模块
    QThread *m_ipThread = nullptr;
    QThread *m_networkThread = nullptr;
    bool m_isOnline = false;                  //网络是否有效--默认无效
    bool m_isAllowLocate = false;             //是否允许定位--默认不允许定位
    void initConnect();                       //初始化信号槽
    void location();                          //判断定位
    void requestDataColl(QString cityIdList); //请求收藏数据
    QStandardItemModel *m_model = nullptr;
    QStandardItemModel *m_collectModel = nullptr;
    TitleSearchModel *m_titleModel = nullptr;
    TitleSearchModel *m_bottomCollectModel = nullptr;
    QList<LocationData> m_searchResultList;
    void setModel(); //设置model（7x3x2请求一次）：一页21个城市，缓存两页；当鼠标滑动到某一个位置，再去请求下两页
    void setTitleModel(); //设置titlemodel
    void initModel(LocationData m_locationdata, int i);      //设置搜索model
    void initTitleModel(LocationData m_locationdata, int i); //设置搜索model（标题栏搜索）
    void initCollectModel(QString cityId, QString cityName); //设置收藏城市model
    void addNullItemCollectModel(); //添加空白按钮
    void eventTrackingCollectCity(QString cityName);
    void addItemCollectModel(QString cityId, QString cityName); //添加收藏城市
    void initBottomCollectModel(QString cityId, QString cityName); //设置收藏城市model(主界面底部)
    void processingCommand(const QStringList &cmd);          //处理终端命令
    QString processingApi(const QStringList &cmd);           //处理外部命令
    QTimer *m_canProcess = nullptr;                          //限制操作频率，降低cpu占用
    bool coreOperateTooOften();                              //操作过于频繁
    bool apiFunction();                                      //处理api函数
    int m_itemIndex = 0;                                     //收藏城市model
    int m_BottomItemIndex = 0;                               //收藏城市model(主界面底部)
    QStringList m_fullCityList;                              //获取收藏城市列表
    void setCollectGsettings();                              //更新配置文件中的收藏城市
    bool m_open = false; //判断是打开时，初始化model，还是从列表中添加的----默认为false：打开时初始化model。
    bool m_isDbusServiceStart = false; // 是否由DBus服务激活启动
    QTimer *m_refreshweather;
    bool m_updateList = false; //默认false时，不进行访问收藏城市数据--避免多次访问
    QString m_currentCityId = "";
    bool m_getBottomModle; // 获取收藏数据计入model
    bool m_getLocationToModel; // 定位本地数据计入Bottommodel
    
    QHash<QString, QStandardItem *> m_hashIdItems;
    QHash<int, QStandardItem *> m_itemsHash;
    int m_count = 0; // 记录当前搜藏列表中的城市个数，+1代表空白item的位置
    QStandardItem * m_locationItem = nullptr;

private slots:
    void networkState(bool isOnline);
    void updateCollect(QList<CollectItemData> collectItemList);
    //    void netTEst();
};

#endif // CORE_H
