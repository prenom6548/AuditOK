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


#ifndef DISCREAD_H
#define DISCREAD_H

#include "discscsi.h"

#include <QObject>
#include <QDBusInterface>
#include <QDBusPendingCall>

const QString DiscBusName = "org.freedesktop.UDisks2";
const QString DiscBusBlock = "org.freedesktop.UDisks2.Block";
const QString DiscBusDrive = "org.freedesktop.UDisks2.Drive";
const QString DiscBusProperty = "org.freedesktop.DBus.Properties";
const QString DiscBusFilesystem = "org.freedesktop.UDisks2.Filesystem";
const QString DiscBusObjectPathPrefix = "/org/freedesktop/UDisks2/block_devices/";

/* 类简介:
 * First,  检测是否插入光盘
 * Second, 检测是否是空盘
 * Third,  检测是否盘片可能已损坏
 * Fourth，检测是否可追加
 * Fifth,  根据是否可追加具体化光盘类型(如不可追加的DVD-R盘的实际类型为DVD-ROM)
 */
class DiscRead:public QObject
{
    friend class MainWindowTest;
    Q_OBJECT
public:
    DiscRead(QString device, QObject* parent=nullptr);
    ~DiscRead();
    bool isReady() const;                   //是否有光盘
    bool isBlank() const;                   //是否是空光盘
    bool isGood() const;                    //光盘是否损坏
    bool isSuccess() const;                 //获取光盘状态失败
    bool canAppend() const;                 //是否可追加
    int numDataTracks() const;
    QString discLabel() const;              //光盘卷标,如果光盘未设置卷标，则该函数返回QSting("");
    QString discMediaType() const;          //光盘介质类型
    QString discFilesystemType() const;     //光盘内部文件系统类型
    void work();                //读取光盘状态信息的入口
    QString fileSystemExtendInf() const;

private:
    void workFromBlock();       //敲定光盘卷标以及文件系统类型信息
    void workFromDrive();       //敲定光盘是否为空以及光盘介质类型
    void workForAppend();       //敲定光盘是否可追加数据、光盘是否可擦除
    void workForIsReady();      //检测光盘是否准备就绪(是否已插入光盘)
    int  readDiscProfile();     //使用scsi具体化光盘介质类型
    void workForMediaType();    //使用udisk2具体化光盘介质类型
    void workForInitMembers();  //初始化成员变量
    void workFromFilesystem();  //检测光盘状态是否良好，是否有损坏
    bool readDiscInformation(uchar** infoData);//调用scsi接口查询光盘信息的入口

Q_SIGNALS:
    void workFinished();
private Q_SLOTS:
    void busPendingCallSlot(QDBusPendingCallWatcher* watcher);//槽函数:查询光盘设备的驱动
private:
    bool mIsGood;               //是否损坏
    bool mIsReady;              //是否插入了光盘
    bool mIsBlank;              //是否是空盘
    uint mProfile;              //scsi光盘介质类型
    bool mCanErase;             //是否可擦除(是否是RW盘)
    bool mCanAppend;            //是否可追加数据
    bool mIsSuccess;            //数据没拿到，判定失败
    int  mNumDataTracks;

    QString mLabel;             //设备卷标，eg "小明的光盘"
    QString mDevice;            //eg "/dev/sr0"
    QString mMediaType;         //eg DVD+RW CD-RW DVD-R
    QString mFilesystemType;    //eg udf iso9660
    QString mFileSystemExtendInf;  //文件系统扩展信息

    QDBusInterface* mDriveInf;
    QDBusInterface* mBlockInf;
    QDBusInterface* mPropertyInf;
    QDBusInterface* mFilesystemInf;
};

QString dvdMediaType(const QString& type);

#endif // DISCREAD_H
