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
#include "coreinteraction.h"
#include <kysdk/diagnosetest/libkydatacollect.h>

Interaction *Interaction::m_interaction(nullptr);

Interaction::~Interaction() {}

Interaction *Interaction::getInstance()
{
    if (m_interaction == nullptr) {
        m_interaction = new CoreInteraction;
    }
    return m_interaction;
}

CoreInteraction::CoreInteraction()
{
    m_canProcess = new QTimer(this);
    m_canProcess->setSingleShot(true);
}

void CoreInteraction::creatCore(const QStringList &list)
{
    qDebug() << " ##### Function Positioning ##### " <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
    //防止多次初始化后端-默认false
    if (m_isCoreInit) {
        return;
    }
    m_core = new Core;
    //线程
    QThread *thread = new QThread();
    //绑定信号和槽
    initConnect();
    //放到线程
    m_core->moveToThread(thread);
    thread->start();
    emit coreInitCore(list); //初始化core
    emit toStartView();  //初始化界面
    m_isCoreInit = true;
}

void CoreInteraction::initConnect()
{
    connect(this, &CoreInteraction::coreInitCore, m_core, &Core::initCore); //初始化core
    connect(ErrorReact::getInstance(), &ErrorReact::netUiShow, this,
            &CoreInteraction::netUiShow); //返回网络错误，给UI发信号
    connect(ErrorReact::getInstance(), &ErrorReact::serverUiShow, this,
            &CoreInteraction::serverUiShow); //返回请求服务器错误，给UI发信号
    connect(ErrorReact::getInstance(), &ErrorReact::dataUiShow, this,
            &CoreInteraction::dataUiShow); //返回数据解析错误，给UI发信号
    connect(ErrorReact::getInstance(), &ErrorReact::netTimeout, this, &CoreInteraction::netTimeout); //超时返回
    connect(ErrorReact::getInstance(), &ErrorReact::geoUiShow, this, &CoreInteraction::geoUiShow);   //定位失败
    connect(m_core, &Core::sigShowBottomList, this, &CoreInteraction::sigShowBottomList); // 收藏城市+定位城市<2时，隐藏列表
    connect(m_core, &Core::searchNull, this, &CoreInteraction::searchNull); //返回搜索为无匹配城市，给UI发信号
    connect(m_core, &Core::searchUiShow, this, &CoreInteraction::searchUiShow); //返回搜索有/输入为空城市结果，给UI发信号
    connect(m_core, &Core::sigTitleSearchUiShow, this,  &CoreInteraction::searchUiShow); //返回搜索有/输入为空城市结果，给UI发信号
    connect(m_core, &Core::startView, this, &CoreInteraction::toStartView); //后端初始化之后，先进行网络诊断，之后初始化前端
    connect(m_core, &Core::showCenter, this, &CoreInteraction::showInCenter); //显示主界面
    connect(m_core, &Core::createTrayIcon, this, &CoreInteraction::createTrayIcon);     //创建托盘
    connect(m_core, &Core::showCurrent, this, &CoreInteraction::showCurrent);     //返回当前城市
    connect(m_core, &Core::showForcast, this, &CoreInteraction::showForcast);     //返回预测一周信息
    connect(m_core, &Core::showHourly, this, &CoreInteraction::showHourly);       //增加逐小时接口---临时用
    connect(m_core, &Core::showLifeStyle, this, &CoreInteraction::showLifeStyle); //返回生活指数信息
    connect(m_core, &Core::collectNull, this, &CoreInteraction::collectNull);     //收藏城市为空
    connect(m_core, &Core::collectShow, this, &CoreInteraction::collectShow); //收藏城市不为空---初始打开时进行发送
    connect(m_core, &Core::coreProgremExit, this, &CoreInteraction::closePro);                      //关闭程序
    connect(m_core, &Core::sigDeleCollectCity, this, &CoreInteraction::sigDeleCollectCity); // 后端处理完成【取消收藏】，前端UI改变
    connect(m_core, &Core::sigAddCollectCity, this, &CoreInteraction::sigAddCollectCity); // 后端处理完成【取消收藏】，前端UI改变
    connect(m_core, &Core::sigBottomModeGet, this, &CoreInteraction::sigBottomModeGet); // 后端处理完成【取消收藏】，前端UI改变

    connect(this, &CoreInteraction::coreChangeCity, m_core, &Core::changeCity);                     //切换城市
    connect(this, &CoreInteraction::corechangeCityFromSearch, m_core, &Core::changeCityFromSearch); //从搜索界面切换城市
    connect(this, &CoreInteraction::coreAddCollectCity, m_core, &Core::addCollectCity); //从搜索界面添加收藏城市
    connect(this, &CoreInteraction::coreSearchResult, m_core, &Core::searchCity); //根据输入字符，匹配本地城市
    connect(this, &CoreInteraction::coreGetCityModel, m_core, &Core::getCityModel,
            Qt::BlockingQueuedConnection); //获取搜索model指针
    connect(this, &CoreInteraction::coreGetTitleModel, m_core, &Core::getCityTitleModel,
            Qt::BlockingQueuedConnection); //获取搜索model指针
    connect(this, &CoreInteraction::coreGetCollectModel, m_core, &Core::getCollectModel,
            Qt::BlockingQueuedConnection); //获取收藏model指针
    connect(this, &CoreInteraction::coreGetBottomCollectModel, m_core, &Core::getBottomCollectModel,
            Qt::BlockingQueuedConnection); //获取收藏model指针
    connect(this, &CoreInteraction::corechangeCityFromCollect, m_core,
            &Core::changeCityFromCollect);                                              //从收藏列表切换城市
    connect(this, &CoreInteraction::coreDelCollectCity, m_core, &Core::delCollectCity); //删除收藏城市列表中的城市
    connect(this, &CoreInteraction::coreLoadCollect, m_core, &Core::loadCollectCity);      //加载收藏城市列表
    connect(this, &CoreInteraction::coreClosePro, m_core, &Core::progremExit);             //项目退出
    connect(this, &CoreInteraction::setAllowLocationYes, m_core, &Core::allowLocationYes); //允许定位-刷新
    connect(this, &CoreInteraction::setAllowLocationNo, m_core, &Core::allowLocationNo);   //不允许定位
    connect(this, &CoreInteraction::refreshCity, m_core, &Core::refreshCity);              //重试等刷新城市函数

    connect(this, &CoreInteraction::sigCoreAddCollectCity, m_core, &Core::addCollectCityFromWidget);   //收藏城市
    connect(this, &CoreInteraction::sigCoreDelCollectCity, m_core, &Core::delCollectCityFromWidget);   //取消收藏
}

bool CoreInteraction::coreOperateTooOften()
{
    if (m_canProcess->isActive()) {
        return true;
    }
    m_canProcess->start(CoreVar::REFRESH_RATE); //刷新间隔
    return false;
}
//在屏幕中间显示
void CoreInteraction::showCenterAfter()
{
    emit showInCenter();
}

void CoreInteraction::closePrograme()
{
    //在程序推出前调用，不需要传入参数
    kdk_dia_upload_program_exit();
    if (coreOperateTooOften()) {
        return;
    }
    emit coreClosePro();
}

void CoreInteraction::allowLocationYes()
{
    emit setAllowLocationYes();
}

void CoreInteraction::allowLocationNo()
{
    emit setAllowLocationNo();
}

void CoreInteraction::refresh()
{
    emit refreshCity();
}

void CoreInteraction::test()
{
    emit toStartView();
}

void CoreInteraction::changeCity(QString cityId)
{
    if (coreOperateTooOften()) {
        return;
    }
    emit coreChangeCity(cityId);
}

void CoreInteraction::searchResult(QString inputText)
{
    emit coreSearchResult(inputText);
}

QStandardItemModel *CoreInteraction::getCityModel()
{
    return coreGetCityModel();
}

QStandardItemModel *CoreInteraction::getTitleModel()
{
    return coreGetTitleModel();
}

QStandardItemModel *CoreInteraction::getCollectModel()
{
    return coreGetCollectModel();
}

QStandardItemModel *CoreInteraction::getBottomCollectModel()
{
    return coreGetBottomCollectModel();
}

void CoreInteraction::changeCityFromTitleSearch(QString cityId)
{
    emit coreChangeCity(cityId);
}

void CoreInteraction::changeCityFromSearch(QModelIndex index)
{
    if (coreOperateTooOften()) {
        return;
    }

    emit corechangeCityFromSearch(index);
}

void CoreInteraction::addCollectCity(QModelIndex index)
{
    if (coreOperateTooOften()) {
        return;
    }
    emit coreAddCollectCity(index);
}

void CoreInteraction::changeCityFromCollect(QModelIndex index)
{
    if (coreOperateTooOften()) {
        return;
    }
    emit corechangeCityFromCollect(index);
}

void CoreInteraction::delCollectCity(QModelIndex index)
{
    if (coreOperateTooOften()) {
        return;
    }
    emit coreDelCollectCity(index);
}
void CoreInteraction::loadCollectCity()
{
    if (coreOperateTooOften()) {
        return;
    }
    emit coreLoadCollect();
}

void CoreInteraction::addCollectCityFromWidget(QString cityId)
{
    emit sigCoreAddCollectCity(cityId);
}

void CoreInteraction::delCollectCityFromWidget(QString cityId)
{
    emit sigCoreDelCollectCity(cityId);
}

void CoreInteraction::getLAndRCityInfo(QString cityIdList)
{
    emit sigLAndRCityInfo(cityIdList);
}
