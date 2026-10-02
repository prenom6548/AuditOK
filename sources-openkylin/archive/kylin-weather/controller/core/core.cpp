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
#include "core.h"
#include "global/viewvar.h"
#include "global/corevar.h"
#include <QDebug>
#include <kysdk/diagnosetest/libkydatacollect.h>

Core::Core() {}

void Core::initCore(const QStringList &args)
{
    qDebug() << " ##### Function Positioning ##### " <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
    if (args.contains("--dbus-service")) {
        m_isDbusServiceStart = true;
    }

    //初始化各部分
    // model/view
    m_model = new QStandardItemModel();
    m_titleModel = new TitleSearchModel();
    m_collectModel = new QStandardItemModel();
    m_bottomCollectModel = new TitleSearchModel();
    // dbus接口用-暂时不用
    //    m_canProcess = new QTimer(this);
    //    m_canProcess->setSingleShot(true);
    //网络模块
    m_network = new Network;
    // ip定位模块
    m_ipLocation = new Iplocation;
    //数据解析
    m_dataParser = new DataParser;
    //数据请求
    m_dataRequest = new DataRequest;
    //搜索模块
    m_searchMarch = new SearchMarch;
    //定时器-30分钟刷新
    m_refreshweather = new QTimer(this);              //定时更新主界面天气
    m_refreshweather->setTimerType(Qt::PreciseTimer); //精密计时,保持毫秒精度

    // 初始化DBus
    m_dbus = new Dbus(this);
    m_dbus->registerService();

    //自定义类型数据注册
    qRegisterMetaType<LifeStyle>("LifeStyle");
    qRegisterMetaType<CurrentWeather>("CurrentWeather");
    qRegisterMetaType<ForecastWeather>("ForecastWeather");
    qRegisterMetaType<LocationData>("LocationData");
    qRegisterMetaType<ItemData>("ItemData");
    qRegisterMetaType<QList<QPersistentModelIndex>>("QList<QPersistentModelIndex>");
    qRegisterMetaType<QAbstractItemModel::LayoutChangeHint>("QAbstractItemModel::LayoutChangeHint");
    qRegisterMetaType<QList<CurrentWeather>>("QList<CurrentWeather>");

    //初始化信号槽
    initConnect();
    //检查网络
    m_network->checkNetWork();
    m_getBottomModle = true;
    m_getLocationToModel = true;
}
//信号槽
void Core::initConnect()
{
    connect(m_network, &Network::effectiveNetwork, this, &Core::networkState);                    //检查网络状态
    connect(m_network, &Network::haveNet, this, &Core::refreshCity);                              //网络从无到有，开始重新请求
    connect(m_dataRequest, &DataRequest::sendData, m_dataParser, &DataParser::dataReply);         //数据reply
    connect(m_dataRequest, &DataRequest::sendCollData, m_dataParser, &DataParser::dataReplyColl); //收藏城市数据reply
    connect(m_dataParser, &DataParser::showCurrent, this, &Core::showCurrent);                    //发送信号，显示生活指数信息
    connect(m_dataParser, &DataParser::showForcast, this, &Core::showForcast);                    //发送信号，显示当前城市信息
    connect(m_dataParser, &DataParser::showLifeStyle, this, &Core::showLifeStyle);                //发送信号，显示城市预测信息
    connect(m_dataParser, &DataParser::showCollectCode, this, &Core::updateCollect);              //更新收藏城市信息
    connect(m_dataParser, &DataParser::showHourly, this, &Core::showHourly);                      //增加逐小时临时调试接口
    connect(m_refreshweather, &QTimer::timeout, this, &Core::refreshCity);                        //设置定时器，每隔半小时刷新
    m_refreshweather->start(12 * (60 * 60) * 1000);                                                    //定时器开启-12h
}

//城市切换--收藏，搜索，热门
void Core::changeCity(QString cityId)
{
    CoreVar::CURRENT_CITY_ID = cityId;  // 更新 全局变量 当前城市ID
    m_currentCityId = cityId;

    if (cityId == "") {
        ErrorReact::getInstance()->collectAddError("切换城市为空");
        return;
    }
    //城市切换-埋点
    kdk::kabase::BuriedPoint buriedPointChangeCity;
    if (buriedPointChangeCity.functionBuriedPoint(kdk::kabase::AppName::KylinWeather,
                                                  kdk::kabase::BuriedPoint::PT::KylinWeatherChangeCity)) {
        qCritical() << "Error : buried point fail ChangeCity!";
    };

    // 检查网络状态，如果无网络则不发送请求，直接显示N/A并弹窗提示
    if (!CoreVar::HAS_NETWORK) {

        // 显示N/A数据到界面
        showNAData();

        // 弹窗提示网络未连接
        ErrorReact::getInstance()->networkFeedBack(false, "无网络");
        return;
    }

    //获取城市数据
    m_dataRequest->dataRequest(cityId);
    qDebug() << " ===== INFO ===== " <<__FILE__<< "," <<__FUNCTION__<< "," <<__LINE__<< " ===== current city type:" << CoreVar::CURRENT_CITY_TYPE ;
}

//从搜索列表改变当前城市
void Core::changeCityFromSearch(QModelIndex index)
{
    ItemData itemda = m_model->itemFromIndex(index)->data(Qt::UserRole).value<ItemData>();
    if (itemda.cityId.isEmpty() || itemda.cityName.isEmpty()) {
        return;
    }    
    
    // 通过搜索列表切换城市 更换 全局变量 当前城市类别 : 其他类别
    if (CoreVar::LOCATION_CITY_ID == itemda.cityId) {
        CoreVar::CURRENT_CITY_TYPE = CityType::Location;
    } else if (CoreVar::CURRENT_COLLECT_LIST.contains(itemda.cityId)) {
        CoreVar::CURRENT_CITY_TYPE = CityType::Collection;
    } else {
        CoreVar::CURRENT_CITY_TYPE = CityType::Other;
    }
    changeCity(itemda.cityId);
}

//加载收藏城市
void Core::loadCollectCity()
{
    QString collectCity;
    QString collectCityGsetting = CoreVar::getSettings("collect-city").toString();
    // 判断当前应用是否开启定位
    if ("1" == CoreVar::getSettings("allow-locate").toString()) {
        collectCity = CoreVar::LOCATION_CITY_ID + "," + collectCityGsetting;
    } else {
        collectCity = collectCityGsetting;
    }
    QStringList listCollectCity = collectCity.split(",");
    m_fullCityList = collectCityGsetting.split(",");
    m_open = false;
    //判断配置文件有内容，发信号给前端改变控件位置
    if (collectCity != "") {
        emit collectShow();
    }
    m_itemsHash.clear();
    m_count = 0;
    m_collectModel->clear();

    // 用循环的方法将配置文件中的收藏城市添加到收藏列表中
    for (int i = 0; i < listCollectCity.length(); i++) {
        if (listCollectCity.at(i) != "") {
            m_updateList = false; //--备用
            m_itemIndex = i;
            m_searchResultList = m_searchMarch->exactMatchCity(listCollectCity.at(i));
            if (m_searchResultList.length() != 0) {
                LocationData m_locationdata = (LocationData)m_searchResultList.at(0);
                initCollectModel(listCollectCity.at(i), m_locationdata.city);
                m_count = i + 1; 
            } else {
                if (i != 0) {
                    m_fullCityList.removeAt(i-1);
                }
                collectCity.remove(listCollectCity.at(i) + ",");
            }
        }
    }
    addNullItemCollectModel();
    // 加载收藏城市列表 更新 全局变量 当前收藏城市列表
    qDebug() << " ===== INFO ===== " <<__FILE__<< "," <<__FUNCTION__<< "," <<__LINE__<< " ===== current collect list size:" << CoreVar::CURRENT_COLLECT_LIST.size();
    CoreVar::CURRENT_COLLECT_LIST = m_fullCityList;
    qDebug() << " ===== INFO ===== " <<__FILE__<< "," <<__FUNCTION__<< "," <<__LINE__<< " ===== current collect list size:" << CoreVar::CURRENT_COLLECT_LIST.size();
    requestDataColl(collectCity);
}

//显示在屏幕中间
void Core::showInCenter()
{
    emit showCenter();
}

//添加收藏城市
void Core::addCollectCity(QModelIndex index)
{
    //获得点击的城市的cityId--不能用cityName来判断，会重名。
    QStandardItem *item = m_model->itemFromIndex(index);
    ItemData itemdata = item->data(Qt::UserRole).value<ItemData>();

    if (itemdata.cityId.isEmpty() || itemdata.cityName.isEmpty() || itemdata.cityId == "") {
        ErrorReact::getInstance()->collectAddError("添加收藏城市异常，数据为空");
        return;
    }
    collectCity(itemdata.cityId, itemdata.cityName);
    if (CoreVar::CURRENT_CITY_ID == itemdata.cityId) {
        CoreVar::CURRENT_CITY_TYPE = CityType::Collection;
    }
    emit sigAddCollectCity();
}

// 主界面点击图标添加收藏城市
void Core::addCollectCityFromWidget(QString cityId)
{
    // 更新 全局变量 当前城市类别
    CoreVar::CURRENT_CITY_TYPE = CityType::Collection;

    // 添加收藏的城市名称 = 当前城市名字
    QString cityName = CoreVar::CURRENT_CITY_NAME;
    
    collectCity(cityId, cityName);
    return;
}

void Core::collectCity(QString cityId, QString cityName)
{
    //遍历收藏城市列表，不可重复添加
    if (!m_fullCityList.isEmpty()) {
        for (QStringList::iterator it = m_fullCityList.begin(); it != m_fullCityList.end(); ++it) {
            if (*it == cityId) { //若收藏城市列表中已有需要添加的城市，则返回
                ErrorReact::getInstance()->collectAddError("收藏城市列表中已有需要添加的城市");
                return;
            }
        }
    }
    //设置将城市加入配置文件
    QString fullCityID = cityId;
    m_fullCityList.append(fullCityID);
    //更新配置文件
    //重新将收藏城市列表写入配置文件
    setCollectGsettings();
    //更改状态
    m_open = true;
    m_updateList = true;

    // 更新列表
    m_fullCityList.clear();
    QString collectCity = CoreVar::getSettings("collect-city").toString();
    QStringList listCollectCity = collectCity.split(",");
    m_fullCityList = listCollectCity;

    // 收藏城市 更新收藏列表 更新 全局变量 当前收藏列表
    CoreVar::CURRENT_COLLECT_LIST = m_fullCityList;

    //设置item，model--用于界面显示
    if (cityName == "" ) {
        cityName = m_searchMarch->matchCityName(cityId);
    }
    initBottomCollectModel(cityId, cityName);
    addItemCollectModel(cityId, cityName);
}

//从收藏城市切换城市
void Core::changeCityFromCollect(QModelIndex index)
{
    CollectItemData itemda = m_collectModel->itemFromIndex(index)->data(Qt::UserRole).value<CollectItemData>();
    if (itemda.cityId.isEmpty() || itemda.cityName.isEmpty()) {
        ErrorReact::getInstance()->collectAddError("收藏城市切换主界面城市错误");
        return;
    }    

    // 收藏城市列表切换城市 更新 全局变量 当前城市名称 和 当前城市ID 当前城市类别
    CoreVar::CURRENT_CITY_ID = itemda.cityId;
    CoreVar::CURRENT_CITY_NAME = itemda.cityName;
    CoreVar::CURRENT_CITY_TYPE = CityType::Collection;

    //切换城市
    changeCity(itemda.cityId);
}

//删除收藏列表中的城市
void Core::delCollectCity(QModelIndex index)
{
    //遍历列表，拿到需要删除的item对应的id值 -更新配置文件
    QString sign;
    if (m_collectModel != nullptr) {
        //获取收藏列表中的cityId
        sign = m_collectModel->item(index.row(), index.column())->data(Qt::UserRole).value<CollectItemData>().cityId;
    }
    //删除选中项--设置一个list存其余各项
    QList<QStandardItem *> itemList;
    m_hashIdItems.clear();
    //遍历当前整个model
    for (int i = 0; i < m_collectModel->rowCount() - 1; i++) {
        QString id = m_collectModel->item(i, 0)->data(Qt::UserRole).value<CollectItemData>().cityId;
        QStandardItem *item = m_collectModel->item(i, 0)->clone();
        //判断为删除项，跳出此次，继续遍历
        if (index.row() == i) {
            continue;
        }
        //将有效项加入qlist
        itemList.append(item);
        //将对应的item记录在m_hashIdItems中便于在后续添加进行操作
         m_hashIdItems.insert(id, item);
    }

    //清空model，重排整个model
    m_collectModel->clear();
    m_count = 0; // model 清空，因此收藏城市个数为0
    //遍历qlist，将有效item重新排列
    for (int i = 0; i < itemList.length(); i++) {
        // m_collectModel->setItem(i / 3, i % 3, itemList.at(i));
        m_collectModel->setItem(i, itemList.at(i));
        m_count = i + 1;
    }
    delBottomCollectCity(sign);
    addNullItemCollectModel();

    //更新配置文件
    if (!m_fullCityList.empty()) {
        int cityIndex = m_fullCityList.indexOf(sign);
        m_fullCityList.removeAt(cityIndex);
        //判断配置文件为空，发送信号给前端
        if (m_fullCityList.length() == 1) {
            emit collectNull();
        }
        setCollectGsettings();
    }

    // 收藏城市 更新收藏列表 更新 全局变量 当前收藏列表 当前城市类型
    CoreVar::CURRENT_COLLECT_LIST = m_fullCityList;
    emit sigDeleCollectCity();
}

//删除主界面底部收藏列表中的城市
void Core::delBottomCollectCity(QString cityId)
{
    // 对取消收藏的城市ID进行查找对应的某一行，将其删去
    for (int i = 0; i < m_bottomCollectModel->rowCount(); i++) {
        QModelIndex index = m_bottomCollectModel->index(i,0);
        QString indexCityId = index.data(TitleSearchModel::CityId).toString();
        if (indexCityId == cityId) {
            // 判断拥有城市ID的某一行的类别，类别是收藏城市还是定位城市
            if (TitleSearchModel::CityType::Collection == index.data(TitleSearchModel::CityStyle).toInt()) {
                m_bottomCollectModel->removeRow(i);
            }
        }
    }
    // 判断当前的model数量，数量小于2时，界面最底部的图标进行隐藏
    emit sigShowBottomList(m_bottomCollectModel->rowCount());
    
}

// 主界面点击取消收藏
void Core::delCollectCityFromWidget(QString cityId)
{
    // 主界面 -> 取消cityID收藏 -> 更改收藏model -> 更改配置文件 -> 更改界面底部图标model
    //遍历model，拿到需要删除的id对应的item值 -更新配置文件
    //删除选中项--设置一个list存其余各项

    QList<QStandardItem *> itemList;
    itemList.clear();
    m_hashIdItems.clear();
    //遍历当前整个model
    for (int i = 0; i < m_collectModel->rowCount() - 1; i++) {
        QString id = m_collectModel->item(i, 0)->data(Qt::UserRole).value<CollectItemData>().cityId;
        QStandardItem *item = m_collectModel->item(i, 0)->clone();
        if ("1" == CoreVar::getSettings("allow-locate").toString()) {
            if (i == 0) {
                //将有效项加入qlist
                itemList.append(item);
                //将对应的item记录在m_hashIdItems中便于在后续添加进行操作
                m_hashIdItems.insert(id, item);
                continue;
            }
        }
        //判断为删除项，跳出此次，继续遍历
        if (cityId == id){
            continue;
        }
        //将有效项加入qlist
        itemList.append(item);
        //将对应的item记录在m_hashIdItems中便于在后续添加进行操作
        m_hashIdItems.insert(id, item);
    }

    //清空model，重排整个model
    m_collectModel->clear();
    m_count = 0; // model 清空，因此收藏城市个数为0
    //遍历qlist，将有效item重新排列
    for (int i = 0; i < itemList.length(); i++) {
        // m_collectModel->setItem(i / 3, i % 3, itemList.at(i));
        m_collectModel->setItem(i, itemList.at(i));
        m_count = i + 1;
    }
    delBottomCollectCity(cityId);
    addNullItemCollectModel();
    
    //更新配置文件
    if (!m_fullCityList.empty()) {
        int cityIndex = m_fullCityList.indexOf(cityId);
        m_fullCityList.removeAt(cityIndex);
        //判断配置文件为空，发送信号给前端
        if (m_fullCityList.length() == 1) {
            emit collectNull();
        }
        setCollectGsettings();
    }

    // 收藏城市 更新收藏列表 更新 全局变量 当前收藏列表 当前城市类别
    CoreVar::CURRENT_COLLECT_LIST = m_fullCityList;
    if (CoreVar::CURRENT_CITY_ID == CoreVar::LOCATION_CITY_ID) {
        CoreVar::CURRENT_CITY_TYPE = CityType::Location;
    } else {
        CoreVar::CURRENT_CITY_TYPE = CityType::Other;
    }

    return;
}

void Core::allowLocationYes()
{
    CoreVar::setSettings("allow-locate", "1"); //改变定位为允许定位
    location();
}

void Core::allowLocationNo()
{
    CoreVar::setSettings("allow-locate", "0"); //不允许定位

//    // 将主界面最底部的列表中的定位item进行更改数据
//    if (m_locationItem == nullptr) {
//        m_locationItem = new QStandardItem;
//        //设置item不可拖拽
//        m_locationItem->setDragEnabled(false);
//        ///整体存取
//        m_locationItem->setData("", TitleSearchModel::CityId);
//        m_locationItem->setData("", TitleSearchModel::CityName);
//        m_locationItem->setData(TitleSearchModel::CityType::Location, TitleSearchModel::CityStyle);
//        //插入model
//        m_bottomCollectModel->appendRow(m_locationItem);
//    } else {
//        ///整体存取
//        m_locationItem->setData("", TitleSearchModel::CityId);
//        m_locationItem->setData("", TitleSearchModel::CityName);
//    }

    //不允许定位时显示城市：北京
    if (CoreVar::CURRENT_CITY_ID == "") {
        CoreVar::CURRENT_CITY_ID = CoreVar::DEFAULT_CITYID;
        m_dataRequest->dataRequest(CoreVar::DEFAULT_CITYID);
        m_currentCityId = CoreVar::DEFAULT_CITYID;
        CoreVar::CURRENT_CITY_ID = m_currentCityId;
    } else {
        m_dataRequest->dataRequest(CoreVar::CURRENT_CITY_ID);
        m_currentCityId = CoreVar::CURRENT_CITY_ID;
    }
    CoreVar::CURRENT_CITY_NAME = m_searchMarch->matchCityName(m_currentCityId);
    CoreVar::LOCATION_CITY_ID = "";

    // 更新获取收藏列表
    m_fullCityList.clear();
    QString collectCity = CoreVar::getSettings("collect-city").toString();
    QStringList listCollectCity = collectCity.split(",");
    m_fullCityList = listCollectCity;
    // 收藏城市 更新收藏列表 更新 全局变量 当前收藏列表
    CoreVar::CURRENT_COLLECT_LIST = m_fullCityList;
    
    if (m_fullCityList.contains(m_currentCityId)) {
        CoreVar::CURRENT_CITY_TYPE = CityType::Collection;
    } else {
        CoreVar::CURRENT_CITY_TYPE = CityType::Other;
    }
}

//定位
void Core::location()
{
    qDebug() << " ##### Function Positioning ##### " <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
    //判断是否允许定位----默认不允许定位
    if ("1" == CoreVar::getSettings("allow-locate").toString()) {
        qDebug() << " ##### Function Positioning ##### getSettings ##### allow-locate=1 ##### " <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
        //定位--地区名
        QString cityName = m_ipLocation->gotoLocation();
        //发现ipip定位偶尔会出现返回乱码--包含http字段
        if (cityName.contains("http")) {
            ErrorReact::getInstance()->geoFail(tr("Seek failed!"));
            //           cityName = m_ipLocation->gotoLocation();
        }
        //定位失败
        if (cityName == "" || cityName.contains("http")) {
            qDebug() << " ##### Function Positioning ##### 定位失败 ##### " <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
            ErrorReact::getInstance()->geoFail(tr("Seek failed!"));
            m_currentCityId = "";
            refreshCity();
            return;
        }
        //搜索本地词库--地区id
        QString cityId = m_searchMarch->setAutomaticCity(cityName);
        CoreVar::LOCATION_CITY_ID = cityId;

        // 如果不是通过DBus服务启动，或者通过DBus服务启动但已经处理过城市切换
        if (!m_isDbusServiceStart) {
            m_dataRequest->dataRequest(cityId);
            m_currentCityId = cityId;

            // 允许定位（定位成功）/ 不允许定位 更新 全局变量 当前城市名称 和 ID 定位当前城市ID
            CoreVar::CURRENT_CITY_ID = m_currentCityId;
            CoreVar::CURRENT_CITY_NAME = m_searchMarch->matchCityName(m_currentCityId);
            CoreVar::CURRENT_CITY_TYPE = CityType::Location;
        } else {
            qDebug() << "DBus service start: Background location finished, cityId:" << cityId;
        }

        //        if (m_locationItem == nullptr) {
//            m_locationItem = new QStandardItem;
//            //设置item不可拖拽
//            m_locationItem->setDragEnabled(false);
//            ///整体存取
//            m_locationItem->setData(CoreVar::CURRENT_CITY_ID, TitleSearchModel::CityId);
//            m_locationItem->setData(CoreVar::CURRENT_CITY_NAME, TitleSearchModel::CityName);
//            m_locationItem->setData(TitleSearchModel::CityType::Location, TitleSearchModel::CityStyle);
//            //设置悬浮提示
//            m_locationItem->setToolTip(CoreVar::CURRENT_CITY_NAME);
//            //插入model
//            m_bottomCollectModel->appendRow(m_locationItem);
//        } else {
//            ///整体存取
//            m_locationItem->setData(CoreVar::CURRENT_CITY_ID, TitleSearchModel::CityId);
//            m_locationItem->setData(CoreVar::CURRENT_CITY_NAME, TitleSearchModel::CityName);
//            //设置悬浮提示
//            m_locationItem->setToolTip(CoreVar::CURRENT_CITY_NAME);
//        }
    } else if ("0" == CoreVar::getSettings("allow-locate").toString()) {
        qDebug() << " ##### Function Positioning ##### getSettings ##### allow-locate=0 ##### " <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
        if(m_isDbusServiceStart || !m_currentCityId.isEmpty()){
            return;
        }
        //不允许定位时
        m_dataRequest->dataRequest(CoreVar::DEFAULT_CITYID);
        m_currentCityId = CoreVar::DEFAULT_CITYID;
        CoreVar::CURRENT_CITY_ID = m_currentCityId;
        CoreVar::CURRENT_CITY_NAME = m_searchMarch->matchCityName(m_currentCityId);
        CoreVar::LOCATION_CITY_ID = "";
        
        // 更新列表
        m_fullCityList.clear();
        QString collectCity = CoreVar::getSettings("collect-city").toString();
        QStringList listCollectCity = collectCity.split(",");
        m_fullCityList = listCollectCity;
        // 收藏城市 更新收藏列表 更新 全局变量 当前收藏列表
        CoreVar::CURRENT_COLLECT_LIST = m_fullCityList;
    
        if (m_fullCityList.contains(m_currentCityId)) {
            CoreVar::CURRENT_CITY_TYPE = CityType::Collection;
        } else {
            CoreVar::CURRENT_CITY_TYPE = CityType::Other;
        }
        return;
    }

    qDebug() << " ===== INFO ===== " <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__ << " ===== location city id:" << CoreVar::LOCATION_CITY_ID;
}

//搜索城市
void Core::searchCity(QString inputText)
{
    //清空模型-清空上次搜索结果
    m_model->clear();
    m_titleModel->clear();
    if (inputText == "") {
        //输入为空，发信号给前端进行处理
        emit searchUiShow(false);
        return;
    }
    //去搜索模块获得搜索结果list
    m_searchResultList = m_searchMarch->exactMatchCity(inputText);
    if (m_searchResultList.isEmpty()) {
        CoreVar::TITLE_SEARCH_LIST_NUM = 1;
        //搜索为空，发信号给前端进行处理
        emit searchNull();
    } else {
        if (CoreVar::CURRENT_TITLE_SEARCH) {
            setTitleModel(); //设置标题栏搜索模型
        } else {
            setModel();//设置搜索模型
        }
    }
}

//请求收藏数据
void Core::requestDataColl(QString cityIdList)
{
    m_dataRequest->dataRequestCollect(cityIdList);
}

//刷新城市
void Core::refreshCity()
{
    if(CoreVar::HAS_NETWORK){
        qDebug() << "refresh city";
        //拿配置文件数据去请求城市，没有就直接使用默认城市北京
        //    m_network->checkNetWork();
        if (m_currentCityId == "") {
            QString currentInfo = CoreVar::getSettings("weather").toString();
            QStringList currentInfoList = currentInfo.split(",");
            if (currentInfoList.length() >= 8) {
                m_currentCityId = currentInfoList.at(1);
            } else {
                m_currentCityId = CoreVar::DEFAULT_CITYID;
            }

            // 定位失败 更新 全局变量 当前城市ID 和 名称 ，定位城市ID
            CoreVar::CURRENT_CITY_ID = m_currentCityId;
            CoreVar::CURRENT_CITY_NAME = m_searchMarch->matchCityName(m_currentCityId);

            if (CoreVar::CURRENT_COLLECT_LIST.contains(m_currentCityId)) {
                CoreVar::CURRENT_CITY_TYPE = CityType::Collection;
            } else {
                CoreVar::CURRENT_CITY_TYPE = CityType::Other;
            }

            if (CoreVar::LOCATION_CITY_ID == m_currentCityId) {
                CoreVar::CURRENT_CITY_TYPE = CityType::Location;
            }
        }

        m_dataRequest->dataRequest(m_currentCityId);

        // 更改定位后的城市信息
        if (m_locationItem == nullptr) {
            QString name = m_searchMarch->matchCityName(CoreVar::DEFAULT_CITYID);
            m_locationItem = new QStandardItem;
            //设置item不可拖拽
            m_locationItem->setDragEnabled(false);
            ///整体存取
            m_locationItem->setData(CoreVar::DEFAULT_CITYID, TitleSearchModel::CityId);
            m_locationItem->setData(name, TitleSearchModel::CityName);
            m_locationItem->setData(TitleSearchModel::CityType::Location, TitleSearchModel::CityStyle);
            //设置悬浮提示
            m_locationItem->setToolTip(name);
            //插入model
            m_bottomCollectModel->appendRow(m_locationItem);
        }
        qDebug() << " ===== INFO ===== " <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__ << "LOCATION_CITY_ID ===== " << CoreVar::LOCATION_CITY_ID;
    }
}

//搜索model
QStandardItemModel *Core::getCityModel()
{
    return m_model;
}

//标题栏搜索搜索model
QStandardItemModel *Core::getCityTitleModel()
{
    return m_titleModel;
}

//收藏model
QStandardItemModel *Core::getCollectModel()
{
    return m_collectModel;
}

//收藏model
QStandardItemModel *Core::getBottomCollectModel()
{
    return m_bottomCollectModel;
}

//设置搜索model数据
void Core::setModel()
{
    //遍历整个搜索结果列表
    for (int i = 0; i < m_searchResultList.size(); i++) {
        // model数据
        ItemData m_itemData;
        //城市id
        m_itemData.cityId = m_searchResultList.at(i).id;
        //城市名
        m_itemData.cityName = m_searchResultList.at(i).city;
        //初始化item
        QStandardItem *item = new QStandardItem;
        //设置不可拖拽
        item->setDragEnabled(false);
        //整体存取
        item->setData(QVariant::fromValue(m_itemData), Qt::UserRole);
        //设置悬浮提示
        item->setToolTip(m_itemData.cityName);
        //插入model
        m_model->setItem(i / 6, i % 6, item);
        // m_model->setItem(i, item);
    }
    emit searchUiShow(true);
}

//构造搜索model
void Core::initModel(LocationData m_locationdata, int i)
{
    // model数据
    ItemData m_itemData;
    //城市id
    m_itemData.cityId = m_locationdata.id;
    //城市名
    m_itemData.cityName = m_locationdata.city;
    //初始化item
    QStandardItem *item = new QStandardItem;
    //设置不可拖拽
    item->setDragEnabled(false);
    //整体存取
    item->setData(QVariant::fromValue(m_itemData), Qt::UserRole);
    //插入model
    m_model->setItem(i / 6, i % 6, item);
    // m_model->setItem(i, item);
}

void Core::setTitleModel()
{
    m_titleModel->clear();
    CoreVar::TITLE_SEARCH_LIST_NUM = m_searchResultList.size();
    //遍历整个搜索结果列表
    for (int i = 0; i < m_searchResultList.size(); i++) {
        initTitleModel(m_searchResultList.at(i), i);
    }
    emit sigTitleSearchUiShow(true);
}

//构造标题栏搜索model
void Core::initTitleModel(LocationData m_locationdata, int i)
{
    QStandardItem *item = new QStandardItem;
    //设置不可拖拽
    item->setDragEnabled(false);
    //整体存取
    //城市id
    item->setData(m_locationdata.id, TitleSearchModel::CityId);
    //城市名
    item->setData(m_locationdata.city, TitleSearchModel::CityName);
    //插入model
    m_titleModel->setItem(i, 0, item);
}

//初始化构造收藏model
void Core::initCollectModel(QString cityId, QString cityName)
{
    //判断是从搜索界面加入收藏城市，需要知道当前共有多少个城市
    if (m_open) {
        m_itemIndex = m_fullCityList.length() - 2;
    }
    // model数据
    CollectItemData m_collectItemData;
    //城市id
    m_collectItemData.cityId = cityId;
    //城市名
    m_collectItemData.cityName = cityName;
    //设置item
    QStandardItem *item = new QStandardItem;
    //设置item不可拖拽
    item->setDragEnabled(false);
    //整体存取
    item->setData(QVariant::fromValue(m_collectItemData), Qt::UserRole);
    item->setToolTip(cityName);

    //设置model的行列
    // m_collectModel->setItem(m_itemIndex / 3, m_itemIndex % 3, item);
    m_collectModel->setItem(m_itemIndex, item);
    // m_collectModel->appendRow(item);
    m_hashIdItems.insert(cityId, item);

    //请求数据---获取温度和阴晴
    if (m_updateList) {
        if (m_collectItemData.cityId == "") {
            return;
        }
        requestDataColl(m_collectItemData.cityId);
    }
}

// 收藏城市-埋点
void Core::eventTrackingCollectCity(QString cityName)
{
    KCustomProperty property[2];
    KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP,KEVENT_CLICK);
    qDebug() << "收藏的城市" << cityName;
    property[0].key = "collectCityName";
    property[0].value = strdup(cityName.toStdString().c_str());
    kdk_dia_append_custom_property(node,property,1);
    kdk_dia_upload_default(node,"collect_city_name_upload_event","collectCityPage");
    kdk_dia_data_free(node);
}

// 添加收藏城市
void Core::addItemCollectModel(QString cityId, QString cityName)
{
    eventTrackingCollectCity(cityName);
    QStandardItem *item;
    m_count += 1; // 由于当前收藏列表为空，收藏一个新的城市
    if (m_hashIdItems.contains("000")) {
        item = m_hashIdItems.value("000");
        m_hashIdItems.remove("000");
    } else {
        initCollectModel(cityId, cityName);
        addNullItemCollectModel();
        return;
    }
    // model数据
    CollectItemData m_collectItemData;
    //城市id
    m_collectItemData.cityId = cityId;
    //城市名
    m_collectItemData.cityName = cityName;
    //设置item不可拖拽
    item->setDragEnabled(false);
    //整体存取
    item->setData(QVariant::fromValue(m_collectItemData), Qt::UserRole);
    item->setToolTip(cityName);
    m_hashIdItems.insert(cityId, item);

    addNullItemCollectModel();
    //请求数据---获取温度和阴晴
    if (m_updateList) {
        if (m_collectItemData.cityId == "") {
            return;
        }
        requestDataColl(m_collectItemData.cityId);
    }
    return;
}

//  添加收藏空item
void Core::addNullItemCollectModel()
{
    QString cityId = "000";
    QString cityName = ViewVar::COLLENTTEXT;
    //判断是从搜索界面加入收藏城市，需要知道当前共有多少个城市
    int num = m_count;
    // model数据
    CollectItemData m_collectItemData;
    //城市id
    m_collectItemData.cityId = cityId;
    //城市名
    m_collectItemData.cityName = cityName;
    //设置item
    QStandardItem *item = new QStandardItem;
    //设置item不可拖拽
    item->setDragEnabled(false);
    //整体存取
    item->setData(QVariant::fromValue(m_collectItemData), Qt::UserRole);
    item->setToolTip(tr("Collection City"));
    m_hashIdItems.insert(cityId, item);
    
    //设置model的行列
    // m_collectModel->setItem(num / 3, num % 3, item);
    m_collectModel->setItem(num, item);
    // m_collectModel->appendRow(item);
    // m_count += 1;

    return;
}

//构造搜索BottomCollectmodel
void Core::initBottomCollectModel(QString cityId, QString cityName)
{
    if (m_getLocationToModel == true) {
        m_getLocationToModel = false;
        //设置item
        QStandardItem *item = new QStandardItem;
        if ("1" == CoreVar::getSettings("allow-locate").toString()) {
            m_currentCityId = CoreVar::DEFAULT_CITYID;
            if (m_locationItem == nullptr) {
                m_locationItem = item;
                ///整体存取
                item->setData(m_currentCityId, TitleSearchModel::CityId);
                item->setData(TitleSearchModel::CityType::Location, TitleSearchModel::CityStyle);
                //设置item不可拖拽
                item->setDragEnabled(false);
                //插入model
                m_bottomCollectModel->appendRow(item);
            } else {
                item = m_locationItem;
            }
            return;
        } else {
            m_currentCityId = "";
            if (m_locationItem == nullptr) {
                m_locationItem = item;
                ///整体存取
                item->setData(m_currentCityId, TitleSearchModel::CityId);
                item->setData(TitleSearchModel::CityType::Location, TitleSearchModel::CityStyle);
                //设置item不可拖拽
                item->setDragEnabled(false);
                //插入model
                m_bottomCollectModel->appendRow(item);
            } else {
                item = m_locationItem;
            }
        }
    }
    //设置item
    QStandardItem *item = new QStandardItem;
    //设置item不可拖拽
    item->setDragEnabled(false);
    ///整体存取
    item->setData(cityId, TitleSearchModel::CityId);
    item->setData(cityName, TitleSearchModel::CityName);
    item->setData(TitleSearchModel::CityType::Collection, TitleSearchModel::CityStyle);
    //设置悬浮提示
    item->setToolTip(cityName);
    //插入model
    m_bottomCollectModel->appendRow(item);
    emit sigShowBottomList(m_bottomCollectModel->rowCount());
}

//初始化dbus
QString Core::initDbus(const QStringList &arguments)
{
    return QString();
}

void Core::processingCommand(const QStringList &cmd)
{
    if (coreOperateTooOften()) {
        return;
    }
    if (cmd[0] == "-change") {
        //调用函数实现功能
        return;
    }
}

QString Core::showCityWeather(const QString &cityName)
{
    qDebug() << "DBus interface call showCityWeather:" << cityName;
    if (cityName.isEmpty()) {
        m_isDbusServiceStart = true;
        if (m_currentCityId.isEmpty()) {
            m_currentCityId = CoreVar::DEFAULT_CITYID;
        }
        changeCity(m_currentCityId);
        emit showCenter();
        return "{\"status\": 0}";
    }

    // 搜索城市ID
    QString cityId = m_searchMarch->setAutomaticCity(cityName);
    if (cityId.isEmpty()) {
        qDebug() << "Invalid city name:" << cityName;
        if (m_isDbusServiceStart) {
            m_isDbusServiceStart = false;
            // 城市名无效，且是DBus启动，则切换到定位城市但不显示界面
            if (!CoreVar::LOCATION_CITY_ID.isEmpty()) {
                changeCity(CoreVar::LOCATION_CITY_ID);
            }
        }
        return "{\"status\": 2}"; //无效城市名
    }

    // 检查网络连接
    if (!CoreVar::HAS_NETWORK) {
        qDebug() << "Switch failed: No network connection";
        if (m_isDbusServiceStart) {
            m_isDbusServiceStart = false;
            // 网络失败，且是DBus启动，则切换到定位城市但不显示界面
            if (!CoreVar::LOCATION_CITY_ID.isEmpty()) {
                changeCity(CoreVar::LOCATION_CITY_ID);
            }
        }
        return "{\"status\": 3}"; //网络错误
    }

    // 切换城市
//    m_isDbusServiceStart = false; // 成功切换，关闭特殊启动标识
    changeCity(cityId);

    // 显示主界面
    emit showCenter();
    m_currentCityId = cityId;
    qDebug() << "Switch success for city:" << cityName << "(" << cityId << ")";
    return "{\"status\": 0}"; //成功
}

QString Core::processingApi(const QStringList &cmd) {
    return QString();
}

void Core::progremExit()
{
    this->deleteLater();
    emit coreProgremExit();
}

//显示N/A数据到界面
void Core::showNAData()
{
    // 创建默认的当前天气数据
    CurrentWeather currentWeather;
    currentWeather.id = CoreVar::CURRENT_CITY_ID;
    currentWeather.city = m_searchMarch->matchCityName(CoreVar::CURRENT_CITY_ID);
    CoreVar::CURRENT_CITY_NAME = currentWeather.city;
    currentWeather.updatetime = "N/A";
    currentWeather.cond_code = "999";
    currentWeather.cond_txt = "N/A";
    currentWeather.hum = "N/A";
    currentWeather.tmp = "N/A";
    currentWeather.wind_dir = "N/A";
    currentWeather.wind_sc = "N/A";
    currentWeather.serveTime = "N/A";
    
    // 创建默认的预报天气数据
    ForecastWeather forecastWeather;
    forecastWeather.first.cond_code_d = "999";
    forecastWeather.first.cond_code_n = "999";
    forecastWeather.first.cond_txt_d = "N/A";
    forecastWeather.first.cond_txt_n = "N/A";
    forecastWeather.first.forcast_date = "N/A";
    forecastWeather.first.wind_dir = "N/A";
    forecastWeather.first.wind_sc = "N/A";
    forecastWeather.first.date = "N/A";
    forecastWeather.first.dateTime = "N/A";
    forecastWeather.first.tmp_max = "N/A";
    forecastWeather.first.tmp_min = "N/A";
    
    // 复制到其他天
    forecastWeather.second = forecastWeather.first;
    forecastWeather.third = forecastWeather.first;
    forecastWeather.fourth = forecastWeather.first;
    forecastWeather.fifth = forecastWeather.first;
    forecastWeather.sixth = forecastWeather.first;
    forecastWeather.seventh = forecastWeather.first;
    
    // 创建默认的生活指数数据
    LifeStyle lifeStyle;
    lifeStyle.air_brf = "N/A";
    lifeStyle.cw_brf = "N/A";
    lifeStyle.drsg_brf = "N/A";
    lifeStyle.flu_brf = "N/A";
    lifeStyle.sport_brf = "N/A";
    lifeStyle.uv_brf = "N/A";
    
    // 创建默认的逐小时天气数据
    QList<Hourly> hourlyList;
    for (int i = 0; i < 24; ++i) {
        Hourly hourly;
        hourly.cond_code = "999";
        hourly.cond_txt = "N/A";
        hourly.time = "N/A";
        hourly.tmp = "N/A";
        hourlyList.append(hourly);
    }
    
    // 发送信号更新界面
    emit showCurrent(currentWeather);
    emit showForcast(forecastWeather);
    emit showLifeStyle(lifeStyle);
    
    QVariant var;
    var.setValue<QList<Hourly>>(hourlyList);
    emit showHourly(var);
}

bool Core::apiFunction() {}

//配置文件存收藏城市
void Core::setCollectGsettings()
{
    QString newStrCityId = "";
    foreach (QString str, m_fullCityList) {
        if (str != "") {
            newStrCityId.append(str);
            newStrCityId.append(",");
        }
    }
    CoreVar::setSettings("collect-city", newStrCityId); //将更新后的列表写入配置文件
}

void Core::networkState(bool isOnline)
{
    //启动
    if (isOnline) {
        CoreVar::HAS_NETWORK = true;
        qDebug() << " ##### Function Positioning ##### effectiveNetwork = true ##### " <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
        //开始定位
        location();
    } else {
        CoreVar::HAS_NETWORK = false;
        qDebug() << " ##### Function Positioning ##### effectiveNetwork = false ##### " <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
        QString code = "999";
        emit createTrayIcon(code); //启动无网络状态只创建托盘，不刷新界面显示
        //无网络或其他错误
        return;
    }
}

//更新收藏城市列表
void Core::updateCollect(QList<CollectItemData> collectItemList)
{
    if (m_updateList) {
        //遍历当前整个model
        for (int i = 0; i < m_collectModel->rowCount() - 1; i++) {
            if ("1" == CoreVar::getSettings("allow-locate").toString()) {
                if (i == 0) {
                    continue;
                }
            }
            QStandardItem *item = m_collectModel->item(i, 0);
            if (item->data(Qt::UserRole).value<CollectItemData>().cityId == collectItemList.at(0).cityId) {
                m_collectModel->item(i, 0)->setData(QVariant::fromValue(collectItemList.at(0)), Qt::UserRole);
                return;
            }
        }
        return;
    }

    //遍历当前整个model
    for (int i = 0; i < m_collectModel->rowCount() - 1; i++) {
        //遍历到最后一行时，对于空item，跳出--不然会发生闪退
        QStandardItem *item = m_collectModel->item(i, 0);
        for (int k = 0; k < collectItemList.length(); k++) {
            if (item->data(Qt::UserRole).value<CollectItemData>().cityId == collectItemList.at(k).cityId) {
                m_collectModel->item(i, 0)->setData(QVariant::fromValue(collectItemList.at(k)), Qt::UserRole);
            }
        }
    }
}

bool Core::coreOperateTooOften()
{
    if (m_canProcess->isActive()) {
        return true;
    }
    m_canProcess->start(CoreVar::REFRESH_RATE); //刷新间隔
    return false;
}
