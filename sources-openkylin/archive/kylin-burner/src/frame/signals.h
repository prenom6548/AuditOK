/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/

#ifndef GLOBALSIGNAL_H
#define GLOBALSIGNAL_H

#include <QObject>
#include <QDir>
#include <QFileInfo>
#include <QMap>
#include <QRect>
#include <QIcon>

class GlobalUserSignal : public QObject
{
    Q_OBJECT

public:
    static GlobalUserSignal* getInstance();

Q_SIGNALS:
    //在【设置】界面中点击缺点调用BusnessSetting::slotsAccept
    //改变mainWindow界面btnBusness按钮的名字和转向udf刻录功能
    void changeBtnBusnessName();
    //在DataMedium中load函数中调用
    //改变DataCDROM中DataSetting中m_SBurnFormat值
    void changBurnFormat();

    //DataMedia光盘挂载后光盘状态确认有错时，通知BurnData的add按钮
    void secondChangeDiscStatus();

    //当有光盘发送该信号，将信号传输到menu界面让其添加擦除信息
    void haveMedium(void *);
    //刻录操作结束并弹出后，通知UI界面去除光盘的相关信息
    void noMedium(void*);

    //使用udisks2修复光盘的状态判断(空盘、追加)
    void changeDiscStatus();

    //LogicMain类发出信号，通知ProcessBurnData类光驱被拔出
    void removeDisc(QString blockName);
    //主题改变
     void iconChange(QIcon);
     // 加载光盘数据初始化burndata界面
     void sigInitCreateISO(void *);

     //退出App信号
     void sigCloseApp();

     // 快捷键
     void sigDeleteByKeyPress(bool);
     void sigShiftPlusNByKeyPress(bool);
     void sigMenuByRightMousePress();
     void sigDeleteAllByKeyPress(bool);
     void sigAddFileByKeyPress(bool);

     //mainwindow在切换光驱选项时，将光驱类对象传递到viewfilterfile界面上
     void sigChangeComboDiskIndex(void*);

     //mainwindow在切换刻录机选项时，将刻录机选项idx传递到面包屑
     void sigChangeComboxBurnerIndex(int);

     //modaldialog向windowtitle发送close的状态是否改变
     void sigChangeCloseEnabel(bool);

     // datacdrom更新了光盘展示信息，通知mainwindow界面更新信息
     void sigChangeDiscDisplayInfo(void*, QString);

    // windowtitle最大化按钮鼠标划入、划出，通知mainwindow显示、隐藏四分屏
     void sigMaxButtonHoverEnter(QRect rect);
     void sigMaxButtonHoverLeave();

public:  //成员函数
    bool getDeviceBusyStatus(QString);
    void setDeviceBusyStatus(QString,bool);
    void removeDevice(QString,bool);
private:
    static GlobalUserSignal* instance;

    explicit GlobalUserSignal(QObject *parent = nullptr);
    ~GlobalUserSignal();
    GlobalUserSignal(const GlobalUserSignal &) = delete;
    GlobalUserSignal &operator = (const GlobalUserSignal &) = delete;

private:
    QMap<QString,bool> mMap;

};


class GlobalCoreSignal : public QObject
{
    Q_OBJECT

public:
    static GlobalCoreSignal* getInstance();


private:
    static GlobalCoreSignal *instance;

    explicit GlobalCoreSignal(QObject *parent = nullptr);
    ~GlobalCoreSignal();
    GlobalCoreSignal(const GlobalCoreSignal &) = delete;
    GlobalCoreSignal &operator = (const GlobalCoreSignal &) = delete;

Q_SIGNALS:

};

class GlobalConfigSignal : public QObject
{
    Q_OBJECT

public:
    static GlobalConfigSignal* getInstance();

    QString configPath;
    QString scannerPath;
    QString scannerImagePath;
    QString scannerTempPath;


private:
    static GlobalConfigSignal *instance;

    explicit GlobalConfigSignal(QObject *parent = nullptr);
    ~GlobalConfigSignal();
    GlobalConfigSignal(const GlobalConfigSignal &) = delete;
    GlobalConfigSignal &operator = (const GlobalConfigSignal &) = delete;

Q_SIGNALS:

};

#define g_user_signal GlobalUserSignal::getInstance()
#define g_core_signal GlobalCoreSignal::getInstance()
#define g_config_signal GlobalConfigSignal::getInstance()

#endif // GLOBALSIGNAL_H

