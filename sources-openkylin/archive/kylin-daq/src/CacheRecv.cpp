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
#include "CacheRecv.h"

CacheRecv::CacheRecv(QObject *parent):CacheBase(parent)
{

}

void CacheRecv::recvUploadInfo(const QString &info)
{
    QList<QString> infoList;
    infoList << info;
    saveUploadInfo(infoList);

    if (dataLinesGeMaxLines(false,m_numOfQuantitativeUploadLines)){
        emit sendUploadInfoNow(qlonglong(m_numOfQuantitativeUploadLines));
    }
}
void CacheRecv::recvCommonInfo(const QString &info)
{
    QList<QString> infoList;
    infoList << info;
    saveCommonInfo(infoList);
}

void CacheRecv::recvUploadEventInfo(const QString &info)
{
    QList<QString> infoList;
    infoList << info;
    saveUploadEventInfo(infoList);

    if (dataLinesGeMaxLines(true,m_numOfQuantitativeUploadLines)){
        emit sendEventUploadInfoNow(qlonglong(m_numOfQuantitativeUploadLines));
    }

}
void CacheRecv::recvTidInfo(const QString &info)
{
    QList<QString> infoList;
    infoList << info;
    saveTidInfo(infoList);
}

void CacheRecv::checkTidInfo(const QString &info, bool &existState)
{
    existState = checkTidExist(info);
}
bool CacheRecv::saveUploadEventInfo(const QList<QString> &info)
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
    state = m_dataBaseInterface->writeRow(s_uploadEventInfoTableName, key, value);
    return state;
}
bool CacheRecv::saveUploadInfo(const QList<QString> &info)
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
        QString deInfoValue;
        QString deStatusInfoValue;
        QList<QVariant> tempValue;
        QString enInfoValue;
        QString statusInfoValue;
        QString enStatusInfoValue;
        // 生成加密信息字符串
        encryptSimple(tempInfo,enInfoValue);
        // decryptSimple(enInfoValue,deInfoValue);
        //	生成加密状态信息
        QJsonObject tempObj;
        tempObj.insert(s_createTimeStampKey,QDateTime::currentDateTime().toMSecsSinceEpoch()/1000);
        tempObj.insert(s_uploadTimesKey,0);
        statusInfoValue = QString(QJsonDocument(tempObj).toJson(QJsonDocument::JsonFormat::Compact));
        encryptSimple(statusInfoValue, enStatusInfoValue);
        // decryptSimple(enStatusInfoValue,deStatusInfoValue);
        tempValue << enInfoValue << enStatusInfoValue;
        // tempValue << deInfoValue << deStatusInfoValue;
        value << tempValue;
    }
    state = m_dataBaseInterface->writeRow(s_uploadInfoTableName, key, value);
    return state;
}

void CacheRecv::saveUploadStateInfo(const QString &packageInfo,const QString &uploadedMessage, const QString &tids,const QString &createTimeStamp,const KylinDaqCommon::UploadStateEnum &uploadState)
{
    qDebug()<<"CacheRecv::saveUploadStateInfo packageInfo="<<packageInfo;
    qDebug()<<"CacheRecv::saveUploadStateInfo uploadedMessage="<<uploadedMessage;
    qDebug()<<"CacheRecv::saveUploadStateInfo tids="<<tids;
    qDebug()<<"CacheRecv::saveUploadStateInfo createTimeStamp="<<createTimeStamp;
    qDebug()<<"CacheRecv::saveUploadStateInfo uploadState="<<uploadState;
    //	检查输入参数
    bool state = false;
    // 输入键：Value1 为 packageInfo 主键哈希，Value6 为 packageInfo 密文
    QList<QString>  key;
    key << s_stateInfoTablePackageInfo << s_stateInfoTableStateInfo << s_stateInfoTableTidInfo << s_stateInfoTableCreateDateTimeInfo << s_stateInfoTableUploadStateInfo << s_stateInfoTablePackageInfoCipher;
    // 输入值
    QList<QList<QVariant>> value;
    QList<QVariant> tempValue;
    QString pkgKey;
    QString enPackageInfo;
    QString enUploadedMessage;
    QString enTids;
    QString enCreateTimesStamp;
    pkgKey = packageInfoKey(packageInfo);
    encryptSimple(packageInfo,enPackageInfo);
    encryptSimple(uploadedMessage,enUploadedMessage);
    encryptSimple(tids,enTids);
    encryptSimple(createTimeStamp,enCreateTimesStamp);
    tempValue<<pkgKey<<enUploadedMessage<<enTids<<enCreateTimesStamp<<uploadState<<enPackageInfo;
    value<<tempValue;
    state = m_dataBaseInterface->replaceRow(s_stateInfoTableName, key, value);
}

void CacheRecv::updateUploadStateInfo(const QString &packageInfo,const QString &uploadedMessage, const QString &tids,const QString &createTimeStamp,const KylinDaqCommon::UploadStateEnum &uploadState)
{
    qDebug()<<"CacheRecv::updateUploadStateInfo packageInfo="<<packageInfo;
    qDebug()<<"CacheRecv::updateUploadStateInfo uploadedMessage="<<uploadedMessage;
    qDebug()<<"CacheRecv::updateUploadStateInfo tids="<<tids;
    qDebug()<<"CacheRecv::updateUploadStateInfo createTimeStamp="<<createTimeStamp;
    qDebug()<<"CacheRecv::updateUploadStateInfo uploadState="<<uploadState;
    //	检查输入参数
    bool state = false;
    // 输入键：UPDATE 路径 packageInfo 不变，只更新 Value2/3/4/5
    QList<QString>  key;
    key << s_stateInfoTableStateInfo << s_stateInfoTableTidInfo << s_stateInfoTableCreateDateTimeInfo << s_stateInfoTableUploadStateInfo;
    QList<QString> constraintKey;
    constraintKey << s_stateInfoTablePackageInfo;
    // 输入值
    QList<QList<QVariant>> constraintValue;
    QList<QList<QVariant>> value;
    QList<QVariant> tempConstraintValue;
    QList<QVariant> tempValue;
    // 输入值
    QString pkgKey;
    QString enUploadedMessage;
    QString enTids;
    QString enCreateTimesStamp;
    pkgKey = packageInfoKey(packageInfo);
    encryptSimple(uploadedMessage,enUploadedMessage);
    encryptSimple(tids,enTids);
    encryptSimple(createTimeStamp,enCreateTimesStamp);
    tempValue<<enUploadedMessage<<enTids<<enCreateTimesStamp<<uploadState;
    value<<tempValue;
    tempConstraintValue<<pkgKey;
    constraintValue<<tempConstraintValue;
    m_dataBaseInterface->updateRow(s_stateInfoTableName,constraintKey, constraintValue, key, value);
}

bool CacheRecv::saveCommonInfo(const QList<QString> &info)
{
    //	检查输入参数
    bool state = false;
    if (info.size()<=0)
    {
        state = false;
        return state;
    }
    QList<QString>  key;
    key << s_idKey <<s_infoKey ;
    QList<QList<QVariant>> value;
    foreach(const QString &tempInfo,info)
    {
        QList<QVariant> tempValue;
        QString enInfoValue;
        // 生成加密信息字符串
        encryptSimple(tempInfo,enInfoValue);
        tempValue << QVariant(static_cast<qlonglong>(s_commonTablePrimaryKey)) <<enInfoValue;
        value << tempValue;
    }
    qDebug()<<"CacheRecv:common:"<<"key="<<key<<" value="<<value;
    state = m_dataBaseInterface->replaceRow(s_commonInfoTableName, key, value);
    return state;
}

bool CacheRecv::saveTidInfo(const QList<QString> &info)
{
    bool state = false;
    if (info.size()<=0)
    {
        state = false;
        return state;
    }
    QList<QString>  key;
    key << s_infoKey ;
    QList<QList<QVariant>> value;
    foreach(const QString &tempInfo,info)
    {
        QList<QVariant> tempValue;
        QString enInfoValue;
        // 生成哈希键
        enInfoValue = sha256Base64(tempInfo);
        //	生成加密状态信息
        tempValue << enInfoValue ;
        value << tempValue;
    }
    state = m_dataBaseInterface->replaceRow(s_tidInfoTableName, key, value);
    return state;

}

bool CacheRecv::checkTidExist(const QString &info)
{
    qlonglong rowNum;
    QString enInfo;
    enInfo = sha256Base64(info);
    rowNum = m_dataBaseInterface->existRowNum(s_tidInfoTableName, s_infoKey,enInfo);
    if(rowNum > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}


void CacheRecv::getUploadStateMessageInfo(const QString &packageInfo, QString &uploadedMessage, QString &tids, QString &createTimeStamp, KylinDaqCommon::UploadStateEnum &uploadState)
{
    QString pkgKey;
    qDebug()<<"CacheRecv::getUploadStateMessageInfo packageInfo="<<packageInfo;
    QList<QString> key;
    pkgKey = packageInfoKey(packageInfo);
    key << s_stateInfoTablePackageInfo << s_stateInfoTableStateInfo << s_stateInfoTableTidInfo << s_stateInfoTableCreateDateTimeInfo << s_stateInfoTableUploadStateInfo;
    QList<QString> condition;
    condition << QString("%1=\"%2\"").arg(s_stateInfoTablePackageInfo).arg(pkgKey);
    QList<QList<QVariant>>  stateInfo =  m_dataBaseInterface->readRowWhere(s_stateInfoTableName,condition,key,1);
    qDebug()<<"CacheRecv::getUploadStateMessageInfo stateInfo.size()="<<stateInfo.size();
    if (stateInfo.size() == 1)
    {
        // 任一加密字段解密失败：按 packageInfo 哈希删除该行并置空
        QString msgStr;
        QString tidsStr;
        QString tsStr;
        if (!decryptSimple(stateInfo.at(0).at(1).toString(), msgStr)
            || !decryptSimple(stateInfo.at(0).at(2).toString(), tidsStr)
            || !decryptSimple(stateInfo.at(0).at(3).toString(), tsStr)) {
            qWarning() << "CacheRecv::getUploadStateMessageInfo decrypt failed, remove row: table=" << s_stateInfoTableName << "packageInfo=" << packageInfo;
            QList<QVariant> delId;
            delId << pkgKey;
            m_dataBaseInterface->deleteRow(s_stateInfoTableName, {s_stateInfoTablePackageInfo}, {delId});
            uploadState = KylinDaqCommon::UploadStateEnum::UploadStateMessageNull;
            return;
        }
        uploadedMessage = msgStr;
        tids = tidsStr;
        createTimeStamp = tsStr;
        uploadState = KylinDaqCommon::UploadStateEnum(stateInfo.at(0).at(4).toLongLong());
        qDebug()<<"CacheRecv::getUploadStateMessageInfo uploadedMessage="<<uploadedMessage;
        qDebug()<<"CacheRecv::getUploadStateMessageInfo tids="<<tids;
        qDebug()<<"CacheRecv::getUploadStateMessageInfo createTimeStamp="<<createTimeStamp;
        qDebug()<<"CacheRecv::getUploadStateMessageInfo uploadState="<<uploadState;
    }
    else
    {
        uploadState = KylinDaqCommon::UploadStateEnum::UploadStateMessageNull;
    }
}


bool CacheRecv::dataLinesGeMaxLines(bool isEvent,qlonglong maxLines)
{

    qDebug() << "=========================data lines";

    int line = 0;

    if (isEvent){
        line = m_dataBaseInterface->RowNum(s_uploadEventInfoTableName);
    }else {
        line = m_dataBaseInterface->RowNum(s_uploadInfoTableName);
    }


    return line >= maxLines ? true: false;
}

