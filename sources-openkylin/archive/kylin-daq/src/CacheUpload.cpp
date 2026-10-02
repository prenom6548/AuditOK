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
#include "CacheUpload.h"

CacheUpload::CacheUpload(QObject *parent):CacheBase(parent)
{

}

void CacheUpload::recvUploadInfoAndState(const QList<qlonglong> &id,const QList<QString> &info,const QList<QString> & status, const TransmitModule::SendInfoStatus &sendInfoStatus)
{
    if(sendInfoStatus == TransmitModule::SendInfoStatus::OK
        ||sendInfoStatus == TransmitModule::SendInfoStatus::FormatException)
    {
        //
        deleteInfo(s_uploadInfoTableName,id);
    }
    else
    {
        deleteInfo(s_uploadInfoTableName,id);
        saveInfo(s_reuploadInfoTableName,info);
    }
}
void CacheUpload::recvReuploadInfoAndState(const QList<qlonglong> &id,const QList<QString> &info,const QList<QString> & status, const TransmitModule::SendInfoStatus & sendInfoStatus)
{
    if(sendInfoStatus == TransmitModule::SendInfoStatus::OK
        ||sendInfoStatus == TransmitModule::SendInfoStatus::FormatException)
    {
        deleteInfo(s_reuploadInfoTableName,id);
    }
    else
    {
        updateInfo(s_reuploadInfoTableName,id,info,status);
    }
}

void CacheUpload::recvUploadStateInfoAndState(const QList<QString> &packageInfoList,const TransmitModule::SendInfoStatus &sendInfoStatus)
{
    if(sendInfoStatus == TransmitModule::SendInfoStatus::OK
        ||sendInfoStatus == TransmitModule::SendInfoStatus::FormatException)
    {
        updateStateInfo(s_stateInfoTableName,packageInfoList);
    }
}

void CacheUpload::recvUploadSingal(qlonglong sendedInfoNum)
{
    QList<qlonglong> id;
    QList<QString> info;
    QList<QString> status;
    QString requestHead;

    readInfo(s_uploadInfoTableName,sendedInfoNum,id,info,status,requestHead);
    emit sendUploadInfo(id, info, status, requestHead);
}

void CacheUpload::recvReuploadSingal(qlonglong sendedInfoNum)
{
    QList<qlonglong> id;
    QList<QString> info;
    QList<QString> status;
    QString requestHead;
    readInfo(s_reuploadInfoTableName,sendedInfoNum,id,info,status,requestHead);
    emit sendReuploadInfo(id, info, status, requestHead);
}

void CacheUpload::recvUploadStateInfoSingal(qlonglong sendedInfoNum)
{
    QList<QString> packageInfoList;
    QList<QString> uploadedMessageList;
    QList<QString> tidsList;
    QList<QString> createTimeStampList;
    QList<KylinDaqCommon::UploadStateEnum> uploadStateList;
    QString requestHead;
    readStateInfo(s_stateInfoTableName,sendedInfoNum,packageInfoList,uploadedMessageList,tidsList,createTimeStampList,uploadStateList,requestHead);
    emit sendUploadStateInfo(packageInfoList, uploadedMessageList, tidsList, createTimeStampList, uploadStateList, requestHead);
}

bool CacheUpload::saveInfo(const QString &tableName, const QList<QString> &info)
{
    //	检查输入参数
    bool state = false;
    if (info.size()<=0)
    {
        state = false;
        return state;
    }
    QList<QString>  key;
    key << s_infoKey << s_stateKey;
    QList<QList<QVariant>> value;
    foreach(const QString &tempInfo,info)
    {
        QList<QVariant> tempValue;
        QString enInfoValue;
        QString statusInfoValue;
        QString enStatusInfoValue;
        // 生成加密信息字符串
        encryptSimple(tempInfo,enInfoValue);
        //	生成加密状态信息
        QJsonObject tempObj;
        tempObj.insert(s_createTimeStampKey,QDateTime::currentDateTime().toMSecsSinceEpoch()/1000);
        tempObj.insert(s_uploadTimesKey,0);
        statusInfoValue = QString(QJsonDocument(tempObj).toJson(QJsonDocument::JsonFormat::Compact));
        encryptSimple(statusInfoValue, enStatusInfoValue);
        tempValue << enInfoValue << enStatusInfoValue;
        value << tempValue;
    }
    state = m_dataBaseInterface->writeRow(tableName, key, value);
    return state;

}

bool CacheUpload::deleteInfo(const QString &tableName, const QList<qlonglong> &id)
{
    //	检查输入参数
    bool state = false;
    if (id.size()<=0)
    {
        state = false;
        return state;
    }
    QList<QString>  key;
    key << s_idKey;
    QList<QList<QVariant>> value;
    foreach(qlonglong tempInfo,id)
    {
        QList<QVariant> tempValue;
        tempValue << QVariant(tempInfo);
        value << tempValue;
    }
    state = m_dataBaseInterface->deleteRow(tableName, key, value);
    return state;
}

bool CacheUpload::updateInfo(const QString &tableName, const QList<qlonglong> &id, const QList<QString> &info, const QList<QString> &status)
{

    bool state = false;
    QList<QString> constraintKey;
    QList<QList<QVariant>> constraintValue;
    QList<QString> key;
    QList<QList<QVariant>> value;
    qlonglong nowTime = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000;
    constraintKey << s_idKey;
    key << s_infoKey << s_stateKey;
    for(qlonglong index=0;index < id.size();index++)
    {
        QList<QVariant> tempConstraintValue;
        QList<QVariant> tempValue;
        QString enInfo;
        QString enStatus;
        // 检查status信息
        QJsonDocument tempStatusJsonDoc = QJsonDocument::fromJson(status.at(index).toUtf8());
        if(!tempStatusJsonDoc.isObject())
        {
            continue;
        }
        QJsonObject tempStatusJsonObj = tempStatusJsonDoc.object();
        if(!tempStatusJsonObj.contains(s_uploadTimesKey))
        {
            continue;
        }
        tempStatusJsonObj[s_uploadTimesKey] = tempStatusJsonObj[s_uploadTimesKey].toVariant().toLongLong()+1 ;
        qDebug()<<"CacheUpload::updateInfo:tableName="<<tableName<<" index="<<index<<" times="<<tempStatusJsonObj;
        encryptSimple(info.at(index), enInfo);
        encryptSimple(QJsonDocument(tempStatusJsonObj).toJson(QJsonDocument::JsonFormat::Compact), enStatus);
        qlonglong tempId = id.at(index);
        tempConstraintValue << QVariant(tempId);
        tempValue << enInfo << enStatus;
        constraintValue << tempConstraintValue;
        value << tempValue;
    }
    state = m_dataBaseInterface->updateRow(tableName,constraintKey,constraintValue,key,value);
    qDebug()<<"CacheUpload::updateInfo:tableName="<<tableName<<" state="<<state;
    return state;
}

void CacheUpload::readInfo(const QString &tableName, qlonglong sendedInfoNum, QList<qlonglong> &id, QList<QString> &info, QList<QString> &status, QString &requestHead)
{
    QList<QString> uploadInfoKey;
    QList<QString> commonKey;
    uploadInfoKey << s_idKey << s_infoKey << s_stateKey;
    commonKey << s_infoKey;
    // 获取信息
    QList<QList<QVariant>> commonInfoList = m_dataBaseInterface->readRow(s_commonInfoTableName,commonKey,sendedInfoNum);
    if(commonInfoList.size() <= 0)
    {
        return;
    }
    QList<QList<QVariant>> uploadInfoList = m_dataBaseInterface->readRow(tableName,uploadInfoKey,sendedInfoNum);
    if(uploadInfoList.size() <= 0)
    {
        return;
    }
    //  封装信息
    if (!decryptSimple(commonInfoList.first().first().toString(), requestHead)) {
        // 公共信息解密失败：删除该公共行（主键恒为 0，后续 saveCommonInfo 会重建）
        qWarning() << "CacheUpload::readInfo common info decrypt failed, remove common row";
        QList<QList<QVariant>> commonDeleteId;
        QList<QVariant> commonId;
        commonId << QVariant(s_commonTablePrimaryKey);
        commonDeleteId << commonId;
        m_dataBaseInterface->deleteRow(s_commonInfoTableName, {s_idKey}, commonDeleteId);
    }
    //
    QList<qlonglong> failedIds;
    for(int32_t indexNum=0; indexNum < uploadInfoList.size();indexNum++)
    {
        qlonglong rowId = uploadInfoList.at(indexNum).at(0).toLongLong();
        QString infoStr;
        QString statusStr;
        if (!decryptSimple(uploadInfoList.at(indexNum).at(1).toString(), infoStr)
            || !decryptSimple(uploadInfoList.at(indexNum).at(2).toString(), statusStr)) {
            // 任一字段解密失败：删除该行
            qWarning() << "CacheUpload::readInfo decrypt failed, remove row: table=" << tableName << "id=" << rowId;
            failedIds << rowId;
            continue;
        }
        id << rowId;
        info << infoStr;
        status << statusStr;
    }
    if (!failedIds.isEmpty())
        deleteInfo(tableName, failedIds);
}

void CacheUpload::readStateInfo(const QString &tableName, qlonglong sendedInfoNum, QList<QString> &packageInfoList,QList<QString> &uploadedMessageList, QList<QString> &tidsList, QList<QString> &createTimeStampList, QList<KylinDaqCommon::UploadStateEnum> &uploadStateList, QString &requestHead)
{
    QList<QString> uploadInfoKey;
    QList<QString> commonKey;
    QList<QString> conditionList;
    QString condition = QString("%1 != %2").arg(s_stateInfoTableUploadStateInfo).arg(KylinDaqCommon::UploadStateEnum::UploadStateMessageUploaded);
    uploadInfoKey << s_stateInfoTablePackageInfo << s_stateInfoTableStateInfo << s_stateInfoTableTidInfo << s_stateInfoTableCreateDateTimeInfo << s_stateInfoTableUploadStateInfo << s_stateInfoTablePackageInfoCipher;
    commonKey << s_infoKey;
    conditionList << condition; 
    // 获取信息
    QList<QList<QVariant>> commonInfoList = m_dataBaseInterface->readRow(s_commonInfoTableName,commonKey,sendedInfoNum);
    if(commonInfoList.size() <= 0)
    {
        return;
    }
    QList<QList<QVariant>> uploadInfoList = m_dataBaseInterface->readRowWhere(tableName,conditionList,uploadInfoKey,sendedInfoNum);
    if(uploadInfoList.size() <= 0)
    {
        return;
    }
    //  封装信息
    if (!decryptSimple(commonInfoList.first().first().toString(), requestHead)) {
        // 公共信息解密失败：删除该公共行（主键恒为 0，后续 saveCommonInfo 会重建）
        qWarning() << "CacheUpload::readStateInfo common info decrypt failed, remove common row";
        QList<QList<QVariant>> commonDeleteId;
        QList<QVariant> commonId;
        commonId << QVariant(s_commonTablePrimaryKey);
        commonDeleteId << commonId;
        m_dataBaseInterface->deleteRow(s_commonInfoTableName, {s_idKey}, commonDeleteId);
    }
    //
    QList<QList<QVariant>> failedDeleteIds;
    for(int32_t indexNum=0; indexNum < uploadInfoList.size();indexNum++)
    {
        // 状态表以 packageInfo 的哈希作为主键，密文存在 Value6
        const QString pkgKey = uploadInfoList.at(indexNum).at(0).toString();
        QString pkgStr;
        QString msgStr;
        QString tidsStr;
        QString tsStr;
        if (!decryptSimple(uploadInfoList.at(indexNum).at(5).toString(), pkgStr)
            || !decryptSimple(uploadInfoList.at(indexNum).at(1).toString(), msgStr)
            || !decryptSimple(uploadInfoList.at(indexNum).at(2).toString(), tidsStr)
            || !decryptSimple(uploadInfoList.at(indexNum).at(3).toString(), tsStr)) {
            // 任一加密字段解密失败：按主键哈希删除该行
            qWarning() << "CacheUpload::readStateInfo decrypt failed, remove row: table=" << tableName << "packageInfoKey=" << pkgKey;
            QList<QVariant> delId;
            delId << pkgKey;
            failedDeleteIds << delId;
            continue;
        }
        packageInfoList << pkgStr;
        uploadedMessageList << msgStr;
        tidsList << tidsStr;
        createTimeStampList << tsStr;
        uploadStateList << KylinDaqCommon::UploadStateEnum(uploadInfoList.at(indexNum).at(4).toLongLong());
    }
    if (!failedDeleteIds.isEmpty())
        m_dataBaseInterface->deleteRow(tableName, {s_stateInfoTablePackageInfo}, failedDeleteIds);
}

bool CacheUpload::updateStateInfo(const QString &tableName,const QList<QString> &packageInfoList)
{

    bool state = false;
    QList<QString> constraintKey;
    QList<QList<QVariant>> constraintValue;
    QList<QString> key;
    QList<QList<QVariant>> value;
    constraintKey << s_stateInfoTablePackageInfo;
    key <<  s_stateInfoTableUploadStateInfo;
    for(qlonglong index = 0; index < packageInfoList.size(); index++)
    {
        QList<QVariant> tempConstraintValue;
        QList<QVariant> tempValue;
        QString pkgKey;
        qDebug()<<"CacheUpload::updateInfo:tableName="<<tableName<<" index="<<index;
        pkgKey = packageInfoKey(packageInfoList.at(index));

        tempConstraintValue << pkgKey;
        tempValue << KylinDaqCommon::UploadStateEnum::UploadStateMessageUploaded;
        constraintValue << tempConstraintValue;
        value << tempValue;
    }
    state = m_dataBaseInterface->updateRow(tableName,constraintKey,constraintValue,key,value);
    qDebug()<<"CacheUpload::updateInfo:tableName="<<tableName<<" state="<<state;
    return state;
}
void CacheUpload::recvEventUploadInfoAndState(const QList<qlonglong> &id,const QList<QString> &info,const QList<QString> & status, const TransmitModule::SendInfoStatus &sendInfoStatus)
{

    if(sendInfoStatus == TransmitModule::SendInfoStatus::OK
        ||sendInfoStatus == TransmitModule::SendInfoStatus::FormatException)
    {
        deleteInfo(s_uploadEventInfoTableName,id);
    }
    else
    {
        deleteInfo(s_uploadEventInfoTableName,id);
        saveInfo(s_reuploadEventInfoTableName,info);
    }

}
void CacheUpload::recvEventReuploadInfoAndState(const QList<qlonglong> &id,const QList<QString> &info,const QList<QString> & status, const TransmitModule::SendInfoStatus & sendInfoStatus)
{
    if(sendInfoStatus == TransmitModule::SendInfoStatus::OK
        ||sendInfoStatus == TransmitModule::SendInfoStatus::FormatException)
    {
        deleteInfo(s_reuploadEventInfoTableName,id);
    }
    else
    {
        updateInfo(s_reuploadEventInfoTableName,id,info,status);
    }
}
void CacheUpload::recvEventUploadSingal(qlonglong sendedInfoNum)
{
    QList<qlonglong> id;
    QList<QString> info;
    QList<QString> status;
    QString requestHead;

    readInfo(s_uploadEventInfoTableName,sendedInfoNum,id,info,status,requestHead);
    emit sendEventUploadInfo(id, info, status, requestHead);
}

void CacheUpload::recvEventReuploadSingal(qlonglong sendedInfoNum)
{
    QList<qlonglong> id;
    QList<QString> info;
    QList<QString> status;
    QString requestHead;
    readInfo(s_reuploadEventInfoTableName,sendedInfoNum,id,info,status,requestHead);
    emit sendEventReuploadInfo(id, info, status, requestHead);
}
