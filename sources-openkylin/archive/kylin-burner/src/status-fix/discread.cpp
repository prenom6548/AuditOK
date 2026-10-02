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

#include "discread.h"
#include "src/data/deviceglobals.h"
#include "src/data/mmc.h"
#include <QDebug>
#include <QDBusReply>
#include <QDBusConnection>
#include <QDBusObjectPath>
#include <QThread>

DiscRead::DiscRead(QString device, QObject* parent):
    QObject(parent),
    mIsGood(false),
    mIsReady(false),
    mIsBlank(false),
    mProfile(0),
    mCanErase(false),
    mCanAppend(false),
    mIsSuccess(true),
    mNumDataTracks(0),
    mDevice(device),
    mDriveInf(nullptr),
    mBlockInf(nullptr),
    mPropertyInf(nullptr),
    mFilesystemInf(nullptr)
{
    //work();
}

DiscRead::~DiscRead(){
    delete mBlockInf;
    delete mDriveInf;
    delete mPropertyInf;
    delete mFilesystemInf;

    mDriveInf = mBlockInf = mPropertyInf = mFilesystemInf = nullptr;
}

void DiscRead::work(){
    QThread::msleep(200);
    mIsSuccess = true;
    //1. 初始化成员变量、检测光盘是否准备就绪
    workForInitMembers();
    //2. 检测是否是空盘
    //workFromDrive();
    //3. 检测盘片是否可能已损坏
    //workFromFilesystem();
    //4. 检测是否可追加
    //workForAppend();
    //5. 具体化光盘类型
    //workForMediaType();
    //6. 获取光盘文件系统格式和卷标
    //workFromBlock();
}

void DiscRead::workForInitMembers(){
#ifndef UNIT_TEST
    mIsGood  = true;    //假设所有的盘均未损坏
    mIsBlank = false;   //非空盘
    mIsReady = false;   //初始时光盘未准备就绪(没有插入光盘或未检测到光盘)
    mCanAppend = false; //假设光盘不可追加
    delete mDriveInf;
    delete mBlockInf;
    delete mPropertyInf;
    delete mFilesystemInf;
    mDriveInf = mBlockInf = mFilesystemInf = mPropertyInf = nullptr;

    //1. 检测光盘是否已插入
    workForIsReady();
    if(mDevice.isEmpty() || !mIsReady) {     //没有设备时不做后续处理
        qDebug() << __func__ << __LINE__ << " workForIsReady没准备好，退出DiscRead isReady = " << mIsReady;
        return;
    }

    QString busDevicePath = mDevice.mid(5);
    //2. 获取Filesystem dbus对象
    mFilesystemInf = new QDBusInterface(DiscBusName,DiscBusObjectPathPrefix+busDevicePath,
                                        DiscBusFilesystem,QDBusConnection::systemBus());

    if(!mFilesystemInf->isValid()){
        qDebug()<<"["<<mDevice<<"] failed to new Filesystem object!";
    }

    //3. 获取Block dbus对象
    mBlockInf = new QDBusInterface(DiscBusName,DiscBusObjectPathPrefix+busDevicePath,
                                   DiscBusBlock,QDBusConnection::systemBus());
    if(!mBlockInf->isValid()){
        qDebug()<<"["<<mDevice<<"] failed to new Block object!";
    }

    //4. 根据Property dbus对象
    mPropertyInf = new QDBusInterface(DiscBusName,DiscBusObjectPathPrefix+busDevicePath,
                                      DiscBusProperty,QDBusConnection::systemBus());
    if(!mPropertyInf->isValid()){
        qDebug()<<"["<<mDevice<<"] failed to new Property object!";
        return;
    }

    //5. 异步获取设备的Drive dbus对象
    QDBusPendingCall propertyCall = mPropertyInf->asyncCall("Get",DiscBusBlock,"Drive");
    QDBusPendingCallWatcher* propertyCallWatcher = new QDBusPendingCallWatcher(propertyCall, this);
    connect(propertyCallWatcher,&QDBusPendingCallWatcher::finished,this,&DiscRead::busPendingCallSlot);
#endif
}

/* 异步回调，用于从 "org.freedesktop.UDisks2.Block"中获取"Drive"属性
 * 进而下一步获取"org.freedesktop.UDisks2.Drive"，确定光盘是否为空
 */
void DiscRead::busPendingCallSlot(QDBusPendingCallWatcher* watcher){
#ifndef UNIT_TEST
    QDBusPendingReply<QDBusVariant> callReplay = *watcher;
    watcher->deleteLater();
    if(callReplay.isError()){
        qDebug()<<"["<<mDevice<<"] failed query dbus object path!";
        qDebug()<<"["<<mDevice<<"]"<<callReplay.error();
        return;
    }

    //获取设备的Drive object path
    QDBusVariant variantReplay = callReplay.argumentAt<0>();
    QDBusObjectPath* busObjectPath =  (QDBusObjectPath*)(variantReplay.variant().data());

    QString discBusPath = busObjectPath->path();
    if(discBusPath.isEmpty()){
        qDebug()<<"["<<mDevice<<"] cannot new Drive object!";
        return;
    }

    //qDebug()<<"["<<mDevice<<"]"<<discBusPath;
    //5. 根据Drive Object Path获取Drive dbus对象
    mDriveInf = new QDBusInterface(DiscBusName,discBusPath,
                                   DiscBusDrive,QDBusConnection::systemBus());

    workFromDrive();
    workFromBlock();
    workFromFilesystem();
    workForAppend();
    Q_EMIT workFinished();
#endif
}

QString DiscRead::fileSystemExtendInf() const
{
    return mFileSystemExtendInf;
}

void DiscRead::workForIsReady(){
    if(mDevice.isEmpty())
        return;

    //检测是否插入了光盘
    DiscScsi scsi(mDevice);
    scsi[0] = GPCMD_TEST_UNIT_READY;//0x00
    scsi[5] = 0;
    mIsReady =  (scsi.transport() == 0) ? true : false ;

    qDebug()<<"["<<mDevice<<"]"<<(mIsReady?"is ready!":"is not ready!");
}

/* 从Drive中获取光盘的类型以及是否是空盘
 * 注释：通过scsi读取光盘信息的方式也可以读取isBlank信息，但是获取到的DVD+RW盘结果不准确
 */
void DiscRead::workFromDrive(){
#ifndef UNIT_TEST
    if (mDevice.isEmpty() || !mIsReady) {
        //没有设备时不做后续处理
        qDebug() << __func__ << __LINE__ << " workFromDrive没准备好，退出 mIsReady = " << mIsReady;
        return;
    }

    if((nullptr != mDriveInf) && mDriveInf->isValid()){
        mIsBlank = mDriveInf->property("OpticalBlank").toBool(); //获取"OpticalBlank"属性值
        mNumDataTracks = mDriveInf->property("OpticalNumDataTracks").toInt();
        mMediaType = mDriveInf->property("Media").toString();
        if(mMediaType.isEmpty()) {
            //获取光盘状态和光盘类型失败，开启第二次查询
            mIsSuccess = false;
        }
        qDebug() << __func__ << __LINE__ << "Udisk2获取的 mIsBlank = " << mIsBlank;
        qDebug() << __func__ << __LINE__ << "Udisk2获取的 mMediaType = " << mMediaType;
        qDebug() << __func__ << __LINE__ << "Udisk2获取的 mNumDataTracks = " << mNumDataTracks;

        if(mIsBlank)
            mIsGood = true;             //假设所有的空光盘均未损坏

        workForMediaType();
    }else
        qDebug()<<"["<<mDevice<<"] cannot query media type and state!";
#endif
}

/* 从Block中获取光盘的卷标与文件系统类型
 */
void DiscRead::workFromBlock(){
#ifndef UNIT_TEST
    if (mDevice.isEmpty() || !mIsReady) {
        //没有设备时不做后续处理
        qDebug() << __func__ << __LINE__ << " workFromBlock没准备好，退出 mIsReady = " << mIsReady;
        return;
    }

    if(mIsBlank)  {
        //空光盘没有卷标与文件系统，不需要查询
        qDebug() <<__func__ << __LINE__ << " 空光盘没有卷标与文件系统，不需要查询 ";
        return;
    }

    if((nullptr==mBlockInf) || !mBlockInf->isValid()) {
        qDebug() << __func__ << __LINE__ << "mBlockInf 没有获取到";
        return;
    }
    mLabel = mBlockInf->property("IdLabel").toString();
    mFilesystemType = mBlockInf->property("IdType").toString();
    mFileSystemExtendInf = mBlockInf->property("IdVersion").toString();
    qDebug() << __func__ << __LINE__ << "Udisk2获取的 mLabel = " << mLabel;
    qDebug() << __func__ << __LINE__ << "Udisk2获取的 mFilesystemType = " << mFilesystemType;
#endif
}

/* 检测盘盘片可能损坏的逻辑:
 * 光盘没有filesystem(文件系统)的两种状态：空盘和已损坏盘
 * 因此这里我们只检测非空盘是否可能已损坏。
 *
 * 注释：内部存有数据的光盘从udisk2的角度看有两种维度: block(块)和filesystem(文件系统)
 */
void DiscRead::workFromFilesystem(){
#ifndef UNIT_TEST
    if (mDevice.isEmpty() || !mIsReady) {
        qDebug() << __func__ << __LINE__ << " workFromFilesystem没准备好，退出 mIsReady = " << mIsReady;
        return;
    }
    qDebug()<<"["<<mDevice<<"] is"<<(mIsBlank? "empty!":"not empty!");
    if(mIsBlank) {
        qDebug() << __func__ << __LINE__ << " workFromFilesystem空盘，退出 mIsBlank = " << mIsBlank;
        return;
    }

    /** Filesystem Object 有效但是无法获取到光盘内文件系统的类型(指udf或iso9660)，
     * 此时可能是以下3种情况：
     *   1)linux未正确识别(在自测过程中确实存在光盘未被系统识别的情况，重新插入即可)
     *   2)udf格式光盘加载较慢(自测过程中使用udf盘可以出现该现象；如果使用GVolumeMonitor的信号检测光盘插入，则应该不存在这种可能)
     *   3)光盘可能已损坏
     * 应对手段：建议重新插入光盘
    */
    qDebug()<<"["<<mDevice<<"]"<<mFilesystemType;
    if(mFilesystemType.isEmpty() && (nullptr !=mFilesystemInf && mFilesystemInf->isValid())){
        mIsGood = false;
        qDebug()<<"["<<mDevice<<"] is not good!";
        qDebug()<<"["<<mDevice<<"] may not be correctly identified or damaged, plaease reinsert it!";
        return;
    }
    qDebug()<<"["<<mDevice<<"] is good!";
    qDebug()<<"["<<mDevice<<"]"<<mLabel;
    qDebug()<<"["<<mDevice<<"]"<<mFilesystemType;
#endif
}

void DiscRead::workForAppend(){
#ifndef UNIT_TEST
    uchar* infoData = nullptr;
    disc_info_t* discInfo = nullptr;

    if (mDevice.isEmpty() || !mIsReady || !mIsGood) {
        //没有设备或设备损坏时不做后续处理
        qDebug() << __func__ << __LINE__ << " workForAppend没准备好，退出 mIsReady = " << mIsReady << " mIsGood = " << mIsGood ;
        return;
    }

    if(readDiscInformation(&infoData)){
        discInfo = (disc_info_t*)infoData;
        if (discInfo) {
            mCanErase = discInfo->erasable;
            qDebug()<<"["<<mDevice<<"]"<<(mCanErase?"can erase!":"can not erase!");
            if(1 == discInfo->status)
                mCanAppend = true;

            free(infoData);
            infoData = nullptr;
        }
    }

    if(mIsBlank) {
        //空盘一般说"首次刻录"或"首次会话"这个概念
        qDebug() << __func__ << __LINE__ << " workForAppend空光盘，退出 mIsBlank = " << mIsBlank ;
        return;                                     //空盘一般不说"是否可追加"这个概念
    }
    /** DVD+RW盘从Disc Information中获取到的mCanAppend可能不准确
     *  需要使用profile修正
    */
    readDiscProfile();
    if( (mProfile & Device::MEDIA_DVD_ALL) && (mProfile & (Device::MEDIA_DVD_PLUS_RW | Device::MEDIA_DVD_RW_OVWR)) ){
    //if( (mProfile & Device::MEDIA_DVD_ALL) && (mProfile & Device::MEDIA_DVD_PLUS_RW) ){
        qDebug()<<"["<<mDevice<<"] fix appendable property with profile for DVD+RW/DVD-RW.";
        mCanAppend |= true;
    }

    /* 目前无法处理udf盘的追加刻录功能，所以设定udf不可追加刻录 */
    if(mFilesystemType=="udf")
        mCanAppend = false;
    qDebug()<<"["<<mDevice<<"]"<<(mCanAppend? "can append!":"can not append!");
#endif
}

/* 从udisk2获取到光盘介质类型信息后，将其转变为用户可读的表达方式
*/
void DiscRead::workForMediaType(){
#ifndef UNIT_TEST
    if(mDevice.isEmpty() || !mIsReady || mMediaType.isEmpty()) {
        qDebug() << __func__ << __LINE__ << " workForAppend空光盘，退出 mIsBlank = " << mIsBlank << " mMediaType = " << mMediaType;
        return;
    }
    //optical_cd_
    //optical_dvd_r
    //optical_dvd_plus_
    //qDebug()<<__func__<<__LINE__<<mMediaType;
    char ch = mMediaType.at(8).toLatin1();
    //char ch2 = mMediaType.at(12).toLatin1();
    switch(ch){
    case 'c':{   //CD盘
        if(mMediaType == "optical_cd_r"){
            mMediaType = "CD-R";
//            if(!mCanAppend)
//                mMediaType = "CD-ROM";
        }else{
            mMediaType = "CD-RW";
        }
    }
        break;
    case 'd':   //DVD盘
        mMediaType = dvdMediaType(mMediaType);
        break;
    default:
        break;
    }
    qDebug()<<"["<<mDevice<<"]"<<mMediaType;
#endif
}

bool DiscRead::readDiscInformation(uchar** infoData){
    uchar* data;
    uint dataLen;
    uchar header[2];
    bool transferRet;

    if(mDevice.isEmpty())
        return false;

    ::memset(header, 0, 2);
    DiscScsi scsi(mDevice);
    scsi[0] = GPCMD_READ_DISC_INFO;//0x51
    scsi[8] = 2;
    scsi[9] = 0;

    dataLen = 0;
    transferRet = scsi.transport(TR_DIR_READ, header, 2);
    if(!transferRet)
        dataLen = Device::from2Byte(header) + 2u;
    else
        qDebug()<<"["<<mDevice<<"] READ DISC INFORMATION length det failed!";

    if(dataLen < 32){
        dataLen = 32;
    }

    data          = (uchar*)malloc(dataLen);
    *infoData     = data;
    scsi[7] = dataLen >> 8;
    scsi[8] = dataLen;

    transferRet = scsi.transport(TR_DIR_READ, data, dataLen);
    if(!transferRet){
        quint16 tmp = Device::from2Byte((const uchar*)data) + 2;
        uchar* newData = (uchar*)realloc(data,qMin<uint>(dataLen,tmp));
        if(newData)         //防止失败时内存泄露
            *infoData = newData;
        return true;
    }else{
        qDebug()<<"["<<mDevice<<"] READ DISC INFORMATION with real length "<<dataLen<<" failed!";
        return false;
    }
}

/* 具体化DVD盘的类型
 */
QString dvdMediaType(const QString& type){
    QString dvdType;

    if(type.isEmpty() || ('d' != type.at(8).toLatin1()))
        return type;

    //DVD-ROM盘不可再追加文件，从udisks2内部读出来该类盘的标识为"optical_dvd"
    if(11==type.length() && (type=="optical_dvd"))
        return QString("DVD-ROM");

    char ch = type.at(12).toLatin1();

    switch(ch){
    case 'p':{  //DVD+R DVD+RW
        if(type == "optical_dvd_plus_r_dl")
            dvdType = "DVD+R DL";
        else if(type == "optical_dvd_plus_r")
            dvdType = "DVD+R";
        else
            dvdType = "DVD+RW";
    }
        break;
    case 'r':{  //DVD-R DVD-RW
        if(type == "optical_dvd_r")
            dvdType = "DVD-R";
        else
            dvdType = "DVD-RW";
    }
        break;
    default:
        dvdType = type;
        break;
    }
    return dvdType;
}

/* profile可分类CD与DVD, 并严格区分 R RW +-R +-RW */
int DiscRead::readDiscProfile(){
#ifndef UNIT_TEST

    bool transferRet;
    uchar profileBuffer[8];
    DiscScsi scsi(mDevice);

    ::memset(profileBuffer,0,8);
    scsi[0] = GPCMD_GET_CONFIGURATION;//0x46
    scsi[1] = 1;
    scsi[8] = 8;
    scsi[9] = 0;

    transferRet = scsi.transport(TR_DIR_READ, profileBuffer, 8);
    if(!transferRet){
        short profile = Device::from2Byte((const uchar*)&profileBuffer[6]);
        switch(profile){
        case 0x00: mProfile = Device::MEDIA_NONE;          break;
        case 0x08: mProfile = Device::MEDIA_CD_ROM;        break;
        case 0x09: mProfile = Device::MEDIA_CD_R;          break;
        case 0x0A: mProfile = Device::MEDIA_CD_RW;         break;
        case 0x10: mProfile = Device::MEDIA_DVD_ROM;       break;
        case 0x11: mProfile = Device::MEDIA_DVD_R_SEQ;     break;
        case 0x12: mProfile = Device::MEDIA_DVD_RAM;       break;
        case 0x13: mProfile = Device::MEDIA_DVD_RW_OVWR;   break;
        case 0x14: mProfile = Device::MEDIA_DVD_RW_SEQ;    break;
        case 0x15: mProfile = Device::MEDIA_DVD_R_DL_SEQ;  break;
        case 0x16: mProfile = Device::MEDIA_DVD_R_DL_JUMP; break;
        case 0x1A: mProfile = Device::MEDIA_DVD_PLUS_RW;   break;
        case 0x1B: mProfile = Device::MEDIA_DVD_PLUS_R;    break;
        case 0x2B: mProfile = Device::MEDIA_DVD_PLUS_R_DL; break;
        default: break;
        }
    }else{
        qDebug()<<"[<<"<<mDevice<<"] GET CONFIGURATION failed.";
    }
    return 0;
#endif
}

bool DiscRead::isBlank() const{
    return mIsBlank;
}

bool DiscRead::isReady() const{
    return mIsReady;
}

bool DiscRead::isGood() const{
    return mIsGood;
}

bool DiscRead::isSuccess() const
{
    return mIsSuccess;
}

bool DiscRead::canAppend() const{
    return mCanAppend;
}

int DiscRead::numDataTracks() const
{
    return mNumDataTracks;
}

QString DiscRead::discLabel() const{
    return mLabel;
}

QString DiscRead::discMediaType() const{
    return mMediaType;
}

QString DiscRead::discFilesystemType() const{
    return mFilesystemType;
}
