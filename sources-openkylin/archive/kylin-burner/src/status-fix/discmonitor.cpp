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

#include "discmonitor.h"
#include "src/po/translation.h"
#include "src/frame/logger.h"
#include <QDebug>

/** 类外函数前置声明 */
QStringList queryAllDiscDevice(GVolumeMonitor*);

DiscMonitor::DiscMonitor(QObject *parent) : QObject(parent)
{
    initMembers();
}

DiscMonitor::~DiscMonitor(){
    //释放内存，关闭信号连接，停止移动设备的监听
    if(mVolumeMonitor && G_IS_OBJECT(mVolumeMonitor)){
        g_signal_handler_disconnect(mVolumeMonitor, mDiscDriveAddHandle);
        g_signal_handler_disconnect(mVolumeMonitor, mDiscMediaAddHandle);
        g_signal_handler_disconnect(mVolumeMonitor, mDiscDriveRemoveHandle);
        g_signal_handler_disconnect(mVolumeMonitor, mDiscMediaRemoveHandle);
        g_object_unref(mVolumeMonitor);
        mVolumeMonitor = nullptr;
    }
}

/** 静态函数，用于获取系统上已连接的所有光盘列表(注意是光盘列表，不是光驱列表)
 *  静态函数属于类所有，不需要构造对象，不会走构造函数
 */
QStringList DiscMonitor::allDiscDevices(){
    //静态获取当前系统上连接的所有光盘设备
    QStringList queryDeviceListRet= queryAllDiscDevice(nullptr);
    return queryDeviceListRet;
}

void DiscMonitor::initMembers(){
    //1. 获取操作系统设备监视器
    mVolumeMonitor = g_volume_monitor_get();
    if(!G_IS_VOLUME_MONITOR(mVolumeMonitor))
        return;

    //2. 动态监控光驱、光盘的行为(异步操作)
    mDiscDriveAddHandle = g_signal_connect(mVolumeMonitor, "drive_connected", G_CALLBACK(driveAddCallback), this);
    mDiscDriveRemoveHandle = g_signal_connect(mVolumeMonitor, "drive_disconnected", G_CALLBACK(driveRemoveCallback), this);
    mDiscMediaAddHandle = g_signal_connect(mVolumeMonitor, "volume_added", G_CALLBACK(discAddCallback), this);
    mDiscMediaRemoveHandle = g_signal_connect(mVolumeMonitor, "volume_removed", G_CALLBACK(discRemoveCallback), this);
    mDiscMediaChangeHandle = g_signal_connect(mVolumeMonitor, "volume_changed", G_CALLBACK(discChangeCallback), this);

    //3. 类内维护一套光盘设备列表
    //mDeviceList = queryAllDiscDevice(mVolumeMonitor);
    mDiscMediaList = queryAllDiscDevice(mVolumeMonitor);
}

/** 1. 插入移动设备(光驱、U盘、移动硬盘)会进入该函数
*/
void DiscMonitor::driveAddCallback(GVolumeMonitor* gvm, GDrive* gdrive, DiscMonitor* pThis)
{
#ifndef UNIT_TEST
    qDebug()<<__func__<<__LINE__;
    char* addedDriveInode = g_drive_get_identifier(gdrive, G_DRIVE_IDENTIFIER_KIND_UNIX_DEVICE);
    if(addedDriveInode && !strncmp(addedDriveInode,"/dev/sr",7))    //只关注光驱设备
        Q_EMIT pThis->discDriveAdd(QString(addedDriveInode));       //通知外部有新光驱插入
    g_free(addedDriveInode);
#endif
}

/** 1. 拔掉移动设备(光驱、U盘、移动硬盘)会进入该函数
 */
void DiscMonitor::driveRemoveCallback(GVolumeMonitor* gvm, GDrive* gdrive, DiscMonitor* pThis)
{
#ifndef UNIT_TEST
    qDebug()<<__func__<<__LINE__;
    char* removedDriveInode = g_drive_get_identifier(gdrive, G_DRIVE_IDENTIFIER_KIND_UNIX_DEVICE);
    if(removedDriveInode && !strncmp(removedDriveInode,"/dev/sr",7))//只关注光驱设备
        Q_EMIT pThis->discDriveRemove(QString(removedDriveInode));  //通知外部有光驱移除
    g_free(removedDriveInode);
#endif
}

/** 1. 插入移动设备会进入该函数
 */
void DiscMonitor::discAddCallback(GVolumeMonitor* gvm, GVolume* gvolume, DiscMonitor* pThis)
{
#ifndef UNIT_TEST
    qDebug()<<__func__<<__LINE__;
    char* addedMediaInode = g_volume_get_identifier(gvolume, G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE);
    if(addedMediaInode && !strncmp(addedMediaInode,"/dev/sr",7)){//只关注光盘设备
        pThis->mDiscMediaList.append(QString(addedMediaInode));  //维护内部设备列表
        Q_EMIT pThis->discMediaAdd(QString(addedMediaInode));    //通知外部有新光盘接入
    }
    g_free(addedMediaInode);
#endif
}

/** 1. 弹出移动设备、直接拔掉移动设备 会进入该函数
 */
void DiscMonitor::discRemoveCallback(GVolumeMonitor* gvm, GVolume* gvolume, DiscMonitor* pThis){
#ifndef UNIT_TEST
    char* removedMediaInode = g_volume_get_identifier(gvolume, G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE);
    qDebug()<<__func__<<__LINE__<<removedMediaInode;
    if(removedMediaInode && !strncmp(removedMediaInode,"/dev/sr",7)){//只关注光盘设备
        pThis->mDiscMediaList.removeOne(QString(removedMediaInode)); //维护内部设备列表
        Q_EMIT pThis->discMediaRemove(QString(removedMediaInode));   //通知外部有光盘移除
    }
    g_free(removedMediaInode);
#endif
}

void DiscMonitor::discChangeCallback(GVolumeMonitor* gvm, GVolume* gvolume, DiscMonitor* pThis)
{
#ifndef UNIT_TEST
    char* changedMediaInode = g_volume_get_identifier(gvolume, G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE);
    qDebug()<<__func__<<__LINE__<<changedMediaInode;
    if(changedMediaInode && !strncmp(changedMediaInode,"/dev/sr",7)){//只关注光盘设备

        GMount *gmount = g_volume_get_mount(gvolume);
        Q_UNUSED(gmount);
        char* name = g_volume_get_name(gvolume);

        qDebug() << "valume name = " << name;
        QString sname(name);
        if (sname.contains(_("CD Drive")) || sname.contains(_("CD/DVD Drive"))) {
            // “CD 驱动器”，“CD/DVD 驱动器” 移除光驱
            qDebug() << __LINE__ << __func__ << "检测到 [CD 驱动器]和[CD/DVD 驱动器]字样，光盘弹出了";
            LOGGER->info(QString("(DiscMonitor::discChangeCallback) 检测到 CD Drive(CD 驱动器)和CD/DVD Drive(CD/DVD 驱动器)字样，光盘弹出了"));

            if (pThis->mDiscMediaList.contains(changedMediaInode)) {
                pThis->mDiscMediaList.removeOne(QString(changedMediaInode)); //维护内部设备列表
                Q_EMIT pThis->discMediaRemove(QString(changedMediaInode));   //通知外部有光盘移除
            }
        } else {
            if (pThis->mDiscMediaList.contains(changedMediaInode)) {

            } else {
                pThis->mDiscMediaList.append(QString(changedMediaInode));  //维护内部设备列表
                Q_EMIT pThis->discMediaAdd(QString(changedMediaInode));    //通知外部有新光盘接入

            }
        }

        g_free(name);
    }
    g_free(changedMediaInode);
#endif
}

/** 获取系统上已连接的所有卷设备
 *  g_volume_monitor_get_volumes()会把空光驱也当做有一个volume，因此需要使用scsi修正这个判断
 */
QStringList queryAllDiscDevice(GVolumeMonitor* volumeMonitor){
    GDrive* tmpDrive = nullptr;
    GList* drivesList = nullptr;
    QStringList tmpList;
    bool driveHasMedia = false;
    char* driveIdentifier = nullptr;        //如 /dev/sr0
    GVolumeMonitor *tmpVolumeMonitor = nullptr;

    if(!volumeMonitor)
        tmpVolumeMonitor = g_volume_monitor_get();
    else
        tmpVolumeMonitor = G_VOLUME_MONITOR(g_object_ref(volumeMonitor));

    //1. 获取系统上所有的卷设备(不包含/ /boot /boot/efi)
    if(tmpVolumeMonitor && G_IS_VOLUME_MONITOR(tmpVolumeMonitor)){
        drivesList = g_volume_monitor_get_connected_drives(tmpVolumeMonitor);
        g_object_unref(tmpVolumeMonitor);
        tmpVolumeMonitor = nullptr;
    }
    //2. 筛选保留光盘设备
    GList* p  = drivesList;
    if (!p) {
        return tmpList;
    }
    for(; p!=nullptr; p=p->next){
        tmpDrive = G_DRIVE(p->data);
        driveHasMedia = g_drive_has_media(tmpDrive);
        driveIdentifier = g_drive_get_identifier(tmpDrive, G_DRIVE_IDENTIFIER_KIND_UNIX_DEVICE);
        if(driveIdentifier && !strncmp(driveIdentifier,"/dev/sr",7)){   //筛选光盘类型的驱动
            if(driveHasMedia)                                           //空光驱不保留
                tmpList.append(QString(driveIdentifier));
        }
        g_free(driveIdentifier);
        driveIdentifier = nullptr;
    }
    //3. 释放gio链表
    g_list_free_full(drivesList, g_object_unref);
    return tmpList;
}

/** 仅供调试代码使用：每3秒打印一次系统上连接的所有光盘设备
*/
void DiscMonitor::printDeviceList(){
    int realTimeDeviceCount = mDiscMediaList.count();
    for(int index=0; index<realTimeDeviceCount; ++index)
        qDebug()<<__func__<<__LINE__<<mDiscMediaList.at(index);
    qDebug()<<endl;
}
