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


#ifndef DISCMONITOR_H
#define DISCMONITOR_H

#include <QTimer>
#include <QObject>
#include <QStringList>

#ifdef signals          //undef掉关键字，避免QOBject与Gio关键字冲突
#undef signals
#endif

#include <glib.h>
#include <gio/gio.h>

/** DiscMonitor类提供：
 *  1. 静态获取系统上当前连接的所有光盘设备列表
 *  2. 动态检测光驱的插拔操作
 *  3. 动态检测光盘设备的插入与弹出操作
 *  note: 本类的功能主要采用gio库做处理，但是gio API在root权限下无法得到任何有效数据
 *        关于gio库的替代方案，请参考本git项目下的DiscMonitor.h DiscMonitor.cpp文件
 */
class DiscMonitor : public QObject
{
    friend class DiscMonitorTest;
    Q_OBJECT
public:
    explicit DiscMonitor(QObject *parent = nullptr);
    static QStringList allDiscDevices();        //静态函数，返回系统上所有的光盘设备
    ~DiscMonitor();
private:
    void initMembers();
    /** gio 静态回调函数声明 */
    static void driveAddCallback(GVolumeMonitor*, GDrive*, DiscMonitor*);
    static void driveRemoveCallback(GVolumeMonitor*, GDrive*, DiscMonitor*);
    static void discAddCallback(GVolumeMonitor*, GVolume*, DiscMonitor*);
    static void discRemoveCallback(GVolumeMonitor*, GVolume*, DiscMonitor*);
    static void discChangeCallback(GVolumeMonitor*, GVolume*, DiscMonitor*);
private Q_SLOTS:
    void printDeviceList();


Q_SIGNALS:
    void discMediaAdd(QString discInode);       //用于通知光盘插入的信号
    void discMediaRemove(QString discInode);    //用于通知光盘弹出的信号
    void discDriveAdd(QString driveInode);      //用于通知光驱插入的信号
    void discDriveRemove(QString driveInode);   //用于通知光驱拔出的信号

private:
    GVolumeMonitor* mVolumeMonitor;
    int mDiscMediaAddHandle     =-1;            //监听光盘插入的句柄
    int mDiscMediaRemoveHandle  =-1;            //监听光盘弹出或拔掉的句柄
    int mDiscMediaChangeHandle  =-1;            //监听光盘改变的句柄
    int mDiscDriveAddHandle     =-1;            //监听光驱插入的句柄
    int mDiscDriveRemoveHandle  =-1;            //监听光驱拔掉的句柄
    //QStringList mDiscDriveList;                 //系统上的光驱列表
    QStringList mDiscMediaList;                 //系统上的光盘列表
};

#endif // DISCMONITOR_H
