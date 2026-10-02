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
#ifndef COREINTERACTION_H
#define COREINTERACTION_H
#include <QThread>
#include <QTimer>
#include <global/corevar.h>
#include "controller/core/core.h"
#include "controller/core/errorreact.h"
#include "controller/Interaction.h"
#include <QPersistentModelIndex>
//实际实现功能
class CoreInteraction : public Interaction
{
    Q_OBJECT

signals:
    void coreChangeCity(QString cityId);               //切换城市
    void corechangeCityFromSearch(QModelIndex index);  //从搜索城市列表切换到目标城市
    void corechangeCityFromTitleSearch(QString cityId);  //从标题栏搜索城市列表切换到目标城市
    void coreAddCollectCity(QModelIndex index);        //添加收藏城市
    void corechangeCityFromCollect(QModelIndex index); //从收藏城市切换到目标城市
    void coreDelCollectCity(QModelIndex index);        //删除收藏城市列表中的城市
    void coreSearchResult(QString inputText);          //根据输入，匹配本地城市
    void coreInitCore(const QStringList &args);                               //不能跨线程调用，线程中使用信号槽来实现调用
    void coreLoadCollect();                            //加载收藏城市列表
    void coreshowInCenter();
    void coreClosePro();
    void setAllowLocationYes();
    void setAllowLocationNo();
    void refreshCity();
    void sigCoreChangeCityFromTitleSearch(QString cityid); // 标题栏搜索城市切换城市
    void sigCoreAddCollectCity(QString cityId);            // 主界面点击按钮进行收藏城市
    void sigCoreDelCollectCity(QString cityId);            // 主界面点击按钮进行取消收藏
    void sigLAndRCityInfo(QString cityIdList);
    QStandardItemModel *coreGetCityModel();            //设置搜索城市列表model
    QStandardItemModel *coreGetTitleModel();           //设置标题栏搜索城市列表model
    QStandardItemModel *coreGetCollectModel();         //设置收藏城市列表model
    QStandardItemModel *coreGetBottomCollectModel();   //设置主界面（最低处）收藏城市列表model

public:
    CoreInteraction();

protected:
    virtual void creatCore(const QStringList &list);       //初始化核心模块
    virtual void changeCity(QString cityId);               //切换城市
    virtual void searchResult(QString inputText);          //搜索本地城市进行匹配
    virtual QStandardItemModel *getCityModel();            //获取搜索model指针
    virtual QStandardItemModel *getTitleModel();           //获取标题栏搜索model指针
    virtual QStandardItemModel *getCollectModel();         //获取收藏model指针
    virtual QStandardItemModel *getBottomCollectModel();         //获取主界面（最低处）收藏model指针
    virtual void changeCityFromTitleSearch(QString cityId);  //从标题栏的搜索城市切换到目标城市
    virtual void changeCityFromSearch(QModelIndex index);  //从搜索城市切换到目标城市
    virtual void addCollectCity(QModelIndex index);        //添加收藏城市
    virtual void changeCityFromCollect(QModelIndex index); //从收藏城市切换到目标城市
    virtual void delCollectCity(QModelIndex index);        //删除收藏城市列表中的城市
    virtual void loadCollectCity();                        //加载收藏城市列表
    virtual void showCenterAfter();
    virtual void closePrograme();
    virtual void allowLocationYes();
    virtual void allowLocationNo();
    virtual void refresh();
    virtual void addCollectCityFromWidget(QString cityId);
    virtual void delCollectCityFromWidget(QString cityId);
    virtual void getLAndRCityInfo(QString cityIdList);

private:
    void initConnect();         //初始化绑定
    bool coreOperateTooOften(); //操作过于频繁

    bool m_isCoreInit = false;      //防止多次初始化核心
    QTimer *m_canProcess = nullptr; //限制操作频率，降低cpu占用
    Core *m_core = nullptr;         //核心对象

public slots:
    void test();

    //    QString m_needStartWithOpenImagePath =
    //    "";//UI未初始化时设置，使其初始化完成立即加载图片---可以改成立即加载天气信息
};

#endif // CoreINTERACTIONCoreH
