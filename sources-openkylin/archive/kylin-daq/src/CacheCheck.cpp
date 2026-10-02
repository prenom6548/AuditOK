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
#include "CacheCheck.h"

CacheCheck::CacheCheck(QObject *parent):CacheBase(parent)
{

}

void CacheCheck::checkDataBase(qlonglong storageDuration, qlonglong cacheRows, qlonglong uploadTimes)
{
    //	UploadInfo
    checkTime(s_uploadInfoTableName,storageDuration);
    checkRows(s_uploadInfoTableName,s_idKey,cacheRows);
    //	ReuploadInfo
    checkTime(s_reuploadInfoTableName,storageDuration);
    checkTimes(s_reuploadInfoTableName,uploadTimes);
    checkRows(s_reuploadInfoTableName,s_idKey,cacheRows);
    //	Tid
    checkRows(s_tidInfoTableName,s_infoKey,cacheRows);
}
void CacheCheck::checkTime(QString tableName, qlonglong storageDuration)
{
    QList<QString> key;
    QList<QList<QVariant>> value;
    QList<qlonglong> id;
    QList<QString> status;
    QList<QString> deleteKey;
    QList<QList<QVariant>> deleteId;
    qlonglong nowTime = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000;
    key<<s_idKey<<s_stateKey;
    deleteKey << s_idKey;
    int32_t checkInfoNum = s_checkInfoNum;
    value = m_dataBaseInterface->readRow(tableName,key,checkInfoNum);
    for(int32_t indexNum = 0; indexNum < value.size(); indexNum++)
    {
        qlonglong rowId = value.at(indexNum).at(0).toLongLong();
        QString tempStr;
        if (!decryptSimple(value.at(indexNum).at(1).toString(),tempStr)) {
            // 解密失败：直接将该行加入删除列表
            qWarning() << "CacheCheck::checkTime decrypt failed, remove row: table=" << tableName << "id=" << rowId;
            QList<QVariant> tempDeleteId;
            tempDeleteId << rowId;
            deleteId << tempDeleteId;
            continue;
        }
        id << rowId;
        status << tempStr;
    }
    for(int32_t indexNum=0;indexNum < id.size(); indexNum++)
    {
        QList<QVariant> tempDeleteId;
        QJsonDocument tempJsonDoc = QJsonDocument::fromJson(status.at(indexNum).toUtf8());
        if(!tempJsonDoc.isObject()
                ||	!tempJsonDoc.object().contains(s_createTimeStampKey)
                ||	nowTime - tempJsonDoc.object()[s_createTimeStampKey].toVariant().toLongLong() > storageDuration)
        {
            tempDeleteId << id.at(indexNum);
            deleteId << tempDeleteId;
        }
    }
    qInfo()<<"CacheCheck::checkTime:"<<"tableName="<<tableName<<" storageDuration="<< storageDuration<<" deleteKey="<<deleteKey<<" deleteId="<<deleteId;
    m_dataBaseInterface->deleteRow(tableName,deleteKey,deleteId);

}

void CacheCheck::checkTimes(QString tableName, qlonglong uploadTimes)
{
    QList<QString> key;
    QList<QList<QVariant>> value;
    QList<qlonglong> id;
    QList<QString> status;
    QList<QString> deleteKey;
    QList<QList<QVariant>> deleteId;
    key<<s_idKey<<s_stateKey;
    deleteKey << s_idKey;
    int32_t checkInfoNum = s_checkInfoNum;
    value = m_dataBaseInterface->readRow(tableName,key,checkInfoNum);
    for(int32_t indexNum = 0; indexNum < value.size(); indexNum++)
    {
        qlonglong rowId = value.at(indexNum).at(0).toLongLong();
        QString tempStr;
        if (!decryptSimple(value.at(indexNum).at(1).toString(),tempStr)) {
            // 解密失败：直接将该行加入删除列表
            qWarning() << "CacheCheck::checkTimes decrypt failed, remove row: table=" << tableName << "id=" << rowId;
            QList<QVariant> tempDeleteId;
            tempDeleteId << rowId;
            deleteId << tempDeleteId;
            continue;
        }
        id << rowId;
        status << tempStr;
    }
    for(int32_t indexNum=0;indexNum < id.size(); indexNum++)
    {
        QList<QVariant> tempDeleteId;
        QJsonDocument tempJsonDoc = QJsonDocument::fromJson(status.at(indexNum).toUtf8());
        if(!tempJsonDoc.isObject()
                ||!tempJsonDoc.object().contains(s_uploadTimesKey)
                ||tempJsonDoc.object()[s_uploadTimesKey].toVariant().toLongLong() > uploadTimes
                )
        {
            tempDeleteId << id.at(indexNum);
            deleteId << tempDeleteId;
        }
        if(tempJsonDoc.isObject()
                &&tempJsonDoc.object().contains(s_uploadTimesKey)
                )
        {
           qDebug() <<"CacheCheck::checkTimes:tableName"<<tableName<<" index="<<indexNum<<" times="<<tempJsonDoc.object();
        }
    }
    qInfo()<<"CacheCheck::checkTimes:"<<"tableName="<<tableName<<" uploadTimes="<<uploadTimes<<" deleteKey="<<deleteKey<<" deleteId="<<deleteId;
    m_dataBaseInterface->deleteRow(tableName,deleteKey,deleteId);

}

void CacheCheck::checkRows(QString tableName,QString key,qlonglong cacheRows)
{
    bool state;
    state = m_dataBaseInterface->deleteMaxRow(tableName,key,cacheRows);

    qInfo()<<"CacheCheck::checkRows: tableName="<< tableName<<" cacheRows= "<<cacheRows;
}
