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
#include "CacheModule.h"

QScopedPointer<CacheModule> CacheModule::ms_instance;
CacheModule::CacheModule(QObject *parent)
    : BaseModule(s_moduleName, parent)
    , m_timer(new QTimer(this))
    , m_cacheCheck(new CacheCheck(this))
    , m_cacheCreate(new CacheCreate(this))
    , m_cacheRecv(new CacheRecv(this))
    , m_cacheUpload(new CacheUpload(this))
{
    //	create table
    m_cacheCreate->createTable();
    //
    connect(this,&CacheModule::pollingIntervalChanged,this,&CacheModule::setTimerPollingInterval);
    //
    connect(m_timer, &QTimer::timeout, this, &CacheModule::recvCheckDateBaseSingal);
    connect(m_timer, &QTimer::timeout, m_cacheCreate, &CacheCreate::createTable);
    //
    connect(m_cacheUpload,&CacheUpload::sendUploadInfo,this,&CacheModule::sendUploadInfo);
    connect(m_cacheUpload,&CacheUpload::sendReuploadInfo,this,&CacheModule::sendReuploadInfo);
    connect(m_cacheUpload,&CacheUpload::sendEventUploadInfo,this,&CacheModule::sendEventUploadInfo);
    connect(m_cacheUpload,&CacheUpload::sendEventReuploadInfo,this,&CacheModule::sendEventReuploadInfo);
    connect(m_cacheUpload,&CacheUpload::sendUploadStateInfo,this,&CacheModule::sendUpladStateInfo);

    connect(m_cacheRecv, &CacheRecv::sendUploadInfoNow, this, &CacheModule::recvUploadSignal);
    connect(m_cacheRecv, &CacheRecv::sendEventUploadInfoNow, this, &CacheModule::recvEventUploadSignal);

}

CacheModule::~CacheModule()
{

}

CacheModule *CacheModule::instance()
{
    if (ms_instance.isNull()) {
        ms_instance.reset(new CacheModule());
    }
    return ms_instance.data();
}


void CacheModule::recvUploadInfo(const QString &info)
{
    m_cacheRecv->recvUploadInfo(info);
}

void CacheModule::recvUploadEventInfo(const QString &info)
{
    m_cacheRecv->recvUploadEventInfo(info);
}

void CacheModule::recvCommonInfo(const QString &info)
{
    m_cacheRecv->recvCommonInfo(info);
}

void CacheModule::recvEventUploadInfoAndState(const QList<qlonglong> &id, const QList<QString> &info, const QList<QString> &status, const TransmitModule::SendInfoStatus &sendInfoStatus)
{
    m_cacheUpload->recvEventUploadInfoAndState(id,info,status,sendInfoStatus);
}

void CacheModule::recvEventReuploadInfoAndState(const QList<qlonglong> &id, const QList<QString> &info, const QList<QString> &status, const TransmitModule::SendInfoStatus &sendInfoStatus)
{
    m_cacheUpload->recvEventReuploadInfoAndState(id,info,status,sendInfoStatus);
}
void CacheModule::recvEventUploadSignal(qlonglong num)
{
   m_cacheUpload->recvEventUploadSingal(num);
}

void CacheModule::recvEventReuploadSignal(qlonglong num)
{
    m_cacheUpload->recvEventReuploadSingal(num);
}

void CacheModule::recvTidInfo(const QString &info)
{
    m_cacheRecv->recvTidInfo(info);
}

void CacheModule::recvUploadInfoAndState(const QList<qlonglong> &id, const QList<QString> &info, const QList<QString> &status, const TransmitModule::SendInfoStatus &sendInfoStatus)
{
    m_cacheUpload->recvUploadInfoAndState(id,info,status,sendInfoStatus);
}

void CacheModule::recvReuploadInfoAndState(const QList<qlonglong> &id, const QList<QString> &info, const QList<QString> &status, const TransmitModule::SendInfoStatus &sendInfoStatus)
{
    m_cacheUpload->recvReuploadInfoAndState(id,info,status,sendInfoStatus);
}

void CacheModule::recvStateInfoAndState(const QList<QString> &packageInfoList, const TransmitModule::SendInfoStatus &sendInfoStatus)
{
    m_cacheUpload->recvUploadStateInfoAndState(packageInfoList,sendInfoStatus);
}
void CacheModule::recvUploadSignal(qlonglong num)
{
   m_cacheUpload->recvUploadSingal(num);
}

void CacheModule::recvReuploadSignal(qlonglong num)
{
    m_cacheUpload->recvReuploadSingal(num);
}

void CacheModule::recvUploadStateInfoSingal(qlonglong sendedInfoNum)
{
    m_cacheUpload->recvUploadStateInfoSingal(sendedInfoNum);
}
void CacheModule::checkTidInfo(const QString &info, bool &tidState)
{
    m_cacheRecv->checkTidInfo(info,tidState);
}

void CacheModule::setTimerPollingInterval(qlonglong sec)
{
    m_timer->stop();
    m_timer->setInterval(KylinDaqCommon::secToMsec(sec));
    m_timer->start();
}

void CacheModule::recvCheckDateBaseSingal()
{
    m_cacheCheck->checkDataBase(m_storageDuration,m_cacheRows,m_uploadTimes);
}

void CacheModule::updateProperty(const QJsonObject &conf,const QJsonObject &defaultConf)
{
    qlonglong value;
    KylinDaqCommon::setValidConfInt(conf, defaultConf, moduleName(), s_pollingIntervalKey, value);
    setPollingInterval(value);
    KylinDaqCommon::setValidConfInt(conf, defaultConf, moduleName(), s_storageDurationKey, value);
    setStorageDuration(value);
    KylinDaqCommon::setValidConfInt(conf, defaultConf, moduleName(), s_cacheRowsKey, value);
    setCacheRows(value);
    KylinDaqCommon::setValidConfInt(conf, defaultConf, moduleName(), s_uploadTimesKey, value);
    setUploadTimes(value);
    KylinDaqCommon::setValidConfInt(conf, defaultConf, s_transmitModuleName, s_transmitModuleNumOfQuantitativeUploadLinesKey, value);
    setNumOfQuantitativeUploadLines(value);
}

void CacheModule::printConf()
{
    qDebug()<<"PollintInterval:"<<m_pollingInterval;
    qDebug()<<"CacheSize:"<<m_cacheSize;
    qDebug()<<"StorageDuration:"<<m_storageDuration;
    qDebug()<<"CacheRows:"<<m_cacheRows;
    qDebug()<<"SingleLineBytes:"<<m_singleLineBytes;
    qDebug()<<"UploadTimes:"<<m_uploadTimes;

}

void CacheModule::getUploadStateMessageInfo(const QString &packageInfo, QString &uploadedMessage, QString &tids, QString &createTimeStamp, KylinDaqCommon::UploadStateEnum &uploadState)
{
    m_cacheRecv->getUploadStateMessageInfo(packageInfo, uploadedMessage, tids, createTimeStamp, uploadState);
}

void CacheModule::recvSaveUploadStateInfo(const QString &packageInfo, const QString &uploadedMessage, const QString &tids, const QString &createTimeStamp,const KylinDaqCommon::UploadStateEnum &uploadState)
{

    m_cacheRecv->saveUploadStateInfo(packageInfo, uploadedMessage, tids, createTimeStamp,uploadState);
}

void CacheModule::recvUpdateUploadStateInfo(const QString &packageInfo, const QString &uploadedMessage, const QString &tids, const QString &createTimeStamp,const KylinDaqCommon::UploadStateEnum &uploadState)
{

    m_cacheRecv->updateUploadStateInfo(packageInfo, uploadedMessage, tids, createTimeStamp,uploadState);
}
