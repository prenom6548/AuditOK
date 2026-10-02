/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
#ifndef CACHEUPLOAD_H
#define CACHEUPLOAD_H

#include <QObject>
#include "CacheBase.h"
#ifdef KYLIN_DAQ_FUZZ_LIGHT_TRANSMIT
class TransmitModule
{
public:
    enum SendInfoStatus
    {
        OK = 0,
        FormatException = -1,
        NetworkAnomaly = -2,
        ServerException = -3,
        EncryptMd5Exception = -4,
        ServerReplyException = -5,
        UrlException = -6
    };
};
#else
#include "TransmitModule.h"
#endif


class CacheUpload : public CacheBase
{
    Q_OBJECT
public:
    /**
     * @brief Construct a new Cache Upload object
     * 
     * @param parent 
     */
    CacheUpload(QObject *parent=nullptr);
public slots:
    /**
     * @brief 接收上传的返回结果
     * 
     * @param id 序列号
     * @param info 埋点信息
     * @param status 埋点信息状态
     * @param sendInfoStatus 上传结果
     */
    void recvUploadInfoAndState(const QList<qlonglong> &id,const QList<QString> &info,const QList<QString> & status, const TransmitModule::SendInfoStatus &sendInfoStatus);
    void recvEventUploadInfoAndState(const QList<qlonglong> &id,const QList<QString> &info,const QList<QString> & status, const TransmitModule::SendInfoStatus &sendInfoStatus);
    /**
     * @brief 接收重传的返回结果
     * 
     * @param id 序列号
     * @param info 埋点信息
     * @param status 埋点信息状态
     * @param sendInfoStatus 重传传结果
     */
    void recvReuploadInfoAndState(const QList<qlonglong> &id,const QList<QString> &info,const QList<QString> & status, const TransmitModule::SendInfoStatus &sendInfoStatus);
    void recvEventReuploadInfoAndState(const QList<qlonglong> &id,const QList<QString> &info,const QList<QString> & status, const TransmitModule::SendInfoStatus &sendInfoStatus);
    void recvUploadStateInfoAndState(const QList<QString> &packageInfoList,const TransmitModule::SendInfoStatus &sendInfoStatus);
    /**
     * @brief 接收需要上传数据的数目
     * 
     * @param num 最大上传数据的数目
     */
    void recvUploadSingal(qlonglong sendedInfoNum);
    /**
     * @brief 接收需要重传数据的数目
     * 
     * @param num 最大重传数据的数目
     */
    void recvReuploadSingal(qlonglong sendedInfoNum);
    void recvEventUploadSingal(qlonglong sendedInfoNum);
    void recvEventReuploadSingal(qlonglong sendedInfoNum);
    void recvUploadStateInfoSingal(qlonglong sendedInfoNum);

protected:
    /**
     * @brief 在数据库中存储埋点信息
     *
     * @param tableName 表名
     * @param info 埋点信息
     * @return true 存储成功
     * @return false 存储失败
     */
    bool saveInfo(const QString& tableName,const QList<QString> &info);
    /**
     * @brief 在数据库中删除埋点信息
     *
     * @param tableName 表名
     * @param id 序列号
     * @return true 删除状态
     * @return false 删除状态
     */
    bool deleteInfo(const QString &tableName,const QList<qlonglong> &id);
    /**
     * @brief 在数据库中更新埋点信息
     *
     * @param tableName 表名
     * @param id 序列号
     * @param info 埋点信息
     * @param status 埋点信息状态
     * @return true 更新状态
     * @return false 更新状态
     */
    bool updateInfo(const QString& tableName,const QList<qlonglong> &id,const QList<QString> &info,const QList<QString> & status);
    /**
     * @brief 在数据库中读取埋点信息
     *
     * @param tableName 表名
     * @param sendedInfoNum 读取数据
     * @param id 序列号
     * @param info 埋点信息
     * @param status 埋点信息状态
     * @param requestHead 请求头
     */
    void readInfo(const QString& tableName,qlonglong sendedInfoNum,QList<qlonglong> &id,QList<QString> &info,QList<QString> & status, QString &requestHead);

    void readStateInfo(const QString &tableName, qlonglong sendedInfoNum, QList<QString> &packageInfoList,QList<QString> &uploadedMessageList, QList<QString> &tidsList, QList<QString> &createTimeStampList, QList<KylinDaqCommon::UploadStateEnum> &uploadStateList, QString &requestHead);

    bool updateStateInfo(const QString &tableName,const QList<QString> &packageInfoList);
signals:
    /**
     * @brief 发送待上传数据
     * 
     * @param id 序列号
     * @param info 待上传数据
     * @param status 待上传数据状态
     * @param requestHead 请求头
     */
    void sendUploadInfo(const QList<qlonglong> &id,const QList<QString> &info,const QList<QString> & status, const QString &requestHead);
    /**
     * @brief 发送待重传数据
     * 
     * @param id 序列号
     * @param info 待上传数据
     * @param status 待上传数据状态
     * @param requestHead 请求头
     */
    void sendReuploadInfo(const QList<qlonglong> &id,const QList<QString> &info,const QList<QString> & status, const QString &requestHead);
    void sendEventUploadInfo(const QList<qlonglong> &id,const QList<QString> &info,const QList<QString> & status, const QString &requestHead);
    /**
     * @brief 发送待重传数据
     *
     * @param id 序列号
     * @param info 待上传数据
     * @param status 待上传数据状态
     * @param requestHead 请求头
     */
    void sendEventReuploadInfo(const QList<qlonglong> &id,const QList<QString> &info,const QList<QString> & status, const QString &requestHead);
    void sendUploadStateInfo(const QList<QString> &packageInfoList,const QList<QString> &uploadedMessageList, const QList<QString> &tidsList, const QList<QString> &createTimeStampList, const QList<KylinDaqCommon::UploadStateEnum> uploadStateList,QString &requestHead);
};

#endif // CACHEUPLOAD_H
