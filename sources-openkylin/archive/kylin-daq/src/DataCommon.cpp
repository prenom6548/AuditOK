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
#include "DataCommon.h"

Q_DECLARE_METATYPE(QList<QString>)
Q_DECLARE_METATYPE(TransmitModule::SendInfoStatus)
DataCommon::DataCommon(QObject *parent)
    :QObject(parent)   
{

    createObject();
    connectObject();
    startObject();
}

DataCommon::~DataCommon()
{
    basicInfoModuleThread->quit();
    basicInfoModuleThread->wait();
    basicInfoModuleThread->deleteLater();
    onlineInfoModuleThread->quit();
    onlineInfoModuleThread->wait();
    onlineInfoModuleThread->deleteLater();
    commonInfoModuleThread->quit();
    commonInfoModuleThread->wait();
    commonInfoModuleThread->deleteLater();
    dataCollectionThread->quit();
    dataCollectionThread->wait();
    dataCollectionThread->deleteLater();
    cacheModuleThread->quit();
    cacheModuleThread->wait();
    cacheModuleThread->deleteLater();
    transmitModuleThread->quit();
    transmitModuleThread->wait();
    transmitModuleThread->deleteLater();
    confModuleThread->quit();
    confModuleThread->wait();
    confModuleThread->deleteLater();
}

void DataCommon::createObject()
{
    basicInfoModule = BasicInfoModule::instance();
    onlineInfoModule = OnlineInfoModule::instance();
    commonInfoModule = CommonInfoModule::instance();
    dataCollection = DBusModule::instance();
    cacheModule = CacheModule::instance();
    transmitModule = TransmitModule::instance();
    confModule = ConfModule::instance();

    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    m_timer->setInterval(120*1000);

    basicInfoModuleThread = new QThread(this);
    onlineInfoModuleThread = new QThread();
    commonInfoModuleThread = new QThread(this);
    dataCollectionThread = new QThread(this);
    cacheModuleThread = new QThread(this);
    transmitModuleThread = new QThread(this);
    confModuleThread = new QThread(this);

    basicInfoModule->moveToThread(basicInfoModuleThread);
    onlineInfoModule->moveToThread(onlineInfoModuleThread);
    commonInfoModule->moveToThread(commonInfoModuleThread);
    dataCollection->moveToThread(dataCollectionThread);
    cacheModule->moveToThread(cacheModuleThread);
    transmitModule->moveToThread(transmitModuleThread);
    confModule->moveToThread(confModuleThread);

    qRegisterMetaType<QString>("QString&");
    qRegisterMetaType<QList<QString>>("QList<QString>");
    qRegisterMetaType<TransmitModule::SendInfoStatus>("TransmitModule::SendInfoStatus");
    qRegisterMetaType<KylinDaqCommon::UploadStateEnum>("KylinDaqCommon::UploadStateEnum");
    qRegisterMetaType<KylinDaqCommon::UploadStateEnum>("KylinDaqCommon::UploadStateEnum&");
    qRegisterMetaType<QList<KylinDaqCommon::UploadStateEnum>>("QList<KylinDaqCommon::UploadStateEnum>");
}

void DataCommon::connectObject()
{
    //  connect update property
    connect(confModule,&ConfModule::sendConf,basicInfoModule,&BasicInfoModule::updateProperty);
    connect(confModule,&ConfModule::sendConf,onlineInfoModule,&OnlineInfoModule::updateProperty);
    connect(confModule,&ConfModule::sendConf,commonInfoModule,&CommonInfoModule::updateProperty);
    connect(confModule,&ConfModule::sendConf,dataCollection,&DBusModule::updateProperty);
    connect(confModule,&ConfModule::sendConf,cacheModule,&CacheModule::updateProperty);
    connect(confModule,&ConfModule::sendConf,transmitModule,&TransmitModule::updateProperty);
    connect(confModule,&ConfModule::sendConf,confModule,&ConfModule::updateProperty);


    connect(dataCollection,&DBusModule::saveUploadState,confModule,&ConfModule::writeConf);
    //  save info
    connect(dataCollection, &DBusModule::saveInfo, cacheModule, &CacheModule::recvUploadInfo);
    connect(basicInfoModule, &BasicInfoModule::saveInfo, cacheModule, &CacheModule::recvUploadInfo);
    connect(dataCollection, &DBusModule::saveEventInfo, cacheModule, &CacheModule::recvUploadEventInfo);
    connect(onlineInfoModule, &OnlineInfoModule::saveInfo, cacheModule, &CacheModule::recvUploadInfo);
    connect(commonInfoModule, &CommonInfoModule::saveInfo, cacheModule, &CacheModule::recvUploadInfo);
    connect(commonInfoModule, &CommonInfoModule::sendInfo, cacheModule, &CacheModule::recvCommonInfo);
    //  save tid
    connect(dataCollection, &DBusModule::saveTid, cacheModule, &CacheModule::recvTidInfo);
    //  check tid
    connect(dataCollection, &DBusModule::checkTid, cacheModule, &CacheModule::checkTidInfo,Qt::ConnectionType::DirectConnection);
    //  check state
    connect(dataCollection, &DBusModule::getHisotryStateMessage, cacheModule, &CacheModule::getUploadStateMessageInfo,Qt::ConnectionType::DirectConnection);
    connect(basicInfoModule, &BasicInfoModule::getHisotryStateMessage, cacheModule, &CacheModule::getUploadStateMessageInfo,Qt::ConnectionType::DirectConnection);
    connect(onlineInfoModule, &OnlineInfoModule::getHisotryStateMessage, cacheModule, &CacheModule::getUploadStateMessageInfo,Qt::ConnectionType::DirectConnection);
    connect(commonInfoModule, &CommonInfoModule::getHisotryStateMessage, cacheModule, &CacheModule::getUploadStateMessageInfo,Qt::ConnectionType::DirectConnection);

    //  save state info
    connect(dataCollection, &DBusModule::saveStateMessage,cacheModule, &CacheModule::recvSaveUploadStateInfo);
    connect(basicInfoModule, &BasicInfoModule::saveStateMessage,cacheModule, &CacheModule::recvSaveUploadStateInfo);
    connect(onlineInfoModule, &OnlineInfoModule::saveStateMessage,cacheModule, &CacheModule::recvSaveUploadStateInfo);
    connect(commonInfoModule, &CommonInfoModule::saveStateMessage,cacheModule, &CacheModule::recvSaveUploadStateInfo);
    connect(dataCollection, &DBusModule::updateStateMessage,cacheModule, &CacheModule::recvUpdateUploadStateInfo);
    connect(basicInfoModule, &BasicInfoModule::updateStateMessage,cacheModule, &CacheModule::recvUpdateUploadStateInfo);
    connect(onlineInfoModule, &OnlineInfoModule::updateStateMessage,cacheModule, &CacheModule::recvUpdateUploadStateInfo);
    connect(commonInfoModule, &CommonInfoModule::updateStateMessage,cacheModule, &CacheModule::recvUpdateUploadStateInfo);
    //  upload info
    connect(transmitModule, &TransmitModule::uploadInfoSignal, cacheModule, &CacheModule::recvUploadSignal);
    connect(transmitModule, &TransmitModule::uploadEventInfoSignal, cacheModule, &CacheModule::recvEventUploadSignal);
    connect(transmitModule, &TransmitModule::reuploadEventInfoSignal, cacheModule, &CacheModule::recvEventReuploadSignal);
    connect(transmitModule, &TransmitModule::reuploadInfoSignal, cacheModule, &CacheModule::recvReuploadSignal);
    connect(transmitModule, &TransmitModule::uploadStateInfoSignal, cacheModule, &CacheModule::recvUploadStateInfoSingal);

    connect(cacheModule, &CacheModule::sendUploadInfo, transmitModule, &TransmitModule::uploadInfo);
    connect(cacheModule, &CacheModule::sendReuploadInfo, transmitModule, &TransmitModule::reuploadInfo);
    connect(cacheModule, &CacheModule::sendUpladStateInfo, transmitModule, &TransmitModule::uploadStateInfo);
    connect(cacheModule, &CacheModule::sendEventUploadInfo, transmitModule, &TransmitModule::uploadEventInfo);
    connect(cacheModule, &CacheModule::sendEventReuploadInfo, transmitModule, &TransmitModule::reuploadEventInfo);

    connect(transmitModule, &TransmitModule::retUploadState, cacheModule, &CacheModule::recvUploadInfoAndState);
    connect(transmitModule, &TransmitModule::retEventUploadState, cacheModule, &CacheModule::recvEventUploadInfoAndState);
    connect(transmitModule, &TransmitModule::retEventReuploadState, cacheModule, &CacheModule::recvEventReuploadInfoAndState);
    connect(transmitModule, &TransmitModule::retReuploadState, cacheModule, &CacheModule::recvReuploadInfoAndState);
    connect(transmitModule, &TransmitModule::retUploadStateInfoState, cacheModule, &CacheModule::recvStateInfoAndState);
    //  start connect
    connect(this, &DataCommon::controlCollectInfo, basicInfoModule, &BasicInfoModule::dealInfo);
    connect(this, &DataCommon::controlCollectInfo, onlineInfoModule, &OnlineInfoModule::dealInfo);
    connect(this, &DataCommon::controlCollectInfo, commonInfoModule, &CommonInfoModule::dealInfo);
    // connect(this, &DataCommon::controlCollectInfo, confModule, &ConfModule::getConf);
    // 获取配置
    connect(confModule, &ConfModule::getConf, transmitModule, &TransmitModule::getConfig);
    connect(transmitModule, &TransmitModule::sendConfig, confModule, &ConfModule::writeConf2);
    // 设置120秒后再获取配置
    connect(m_timer,&QTimer::timeout,this,&DataCommon::controlCollectInfo);
}

void DataCommon::startObject()
{
    basicInfoModuleThread->start();
    onlineInfoModuleThread->start();
    commonInfoModuleThread->start();
    dataCollectionThread->start();
    cacheModuleThread->start();
    transmitModuleThread->start();
    confModuleThread->start();
    confModule->readConf(QString()+KylinDaqCommon::gs_cacheFolderPath+KylinDaqCommon::gs_confFileName);
    m_timer->start();
    emit confModule->getConf();
    // emit controlCollectInfo();
}
