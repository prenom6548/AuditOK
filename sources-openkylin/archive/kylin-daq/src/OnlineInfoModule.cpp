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
#include "OnlineInfoModule.h"

QScopedPointer<OnlineInfoModule> OnlineInfoModule::ms_instance;
OnlineInfoModule::OnlineInfoModule(QObject *parent)
    :BaseModule(s_moduleName,parent)
    ,m_timer(new QTimer(this))
{
    connect(m_timer, &QTimer::timeout, this, &OnlineInfoModule::dealInfo);
    connect(this , &OnlineInfoModule::pollingIntervalChanged ,this, &OnlineInfoModule::setTimerPollingInterval);
}

OnlineInfoModule::~OnlineInfoModule()
{

}

OnlineInfoModule *OnlineInfoModule::instance(void)
{

    if (ms_instance.isNull()) {
        ms_instance.reset(new OnlineInfoModule());
    }
    return ms_instance.data();
}

void OnlineInfoModule::dealInfo()
{
    OnlineInfo onlineInfo;
    QScopedPointer<OnlineInfoCollection> onlineInfoCollection(new OnlineInfoCollection(this));
    onlineInfoCollection->getInfo(onlineInfo);
    QJsonObject sourceObj = sourceInfo();
    QJsonObject informationObj = packageOnlineInfo(onlineInfo);
    KylinDaqCommon::UploadStateMessageInfo_t uploadStateMessageInfo;
    KylinDaqCommon::UploadStateMessageInfo_t oldUploadStateMessageInfo;
    createStateMessageInfo(sourceObj,informationObj,uploadStateMessageInfo);
    oldUploadStateMessageInfo.packageInfo = uploadStateMessageInfo.packageInfo;
    getHisotryStateMessage(oldUploadStateMessageInfo.packageInfo,
    oldUploadStateMessageInfo.uploadedMessage,
    oldUploadStateMessageInfo.tids,
    oldUploadStateMessageInfo.createTimeStamp,
    oldUploadStateMessageInfo.uploadState);
    if (oldUploadStateMessageInfo.uploadedMessage != uploadStateMessageInfo.uploadedMessage) 
    {
        saveStateMessageInfo(uploadStateMessageInfo,oldUploadStateMessageInfo,KylinDaqCommon::UploadTypeEnum(m_uploadType));
    }
}

void OnlineInfoModule::setTimerPollingInterval(qlonglong sec)
{
    m_timer->stop();
    m_timer->setInterval(KylinDaqCommon::secToMsec(sec));
    m_timer->start();
}

QJsonObject OnlineInfoModule::packageOnlineInfo(const OnlineInfo &onlineInfo)
{
    QJsonObject onlineInfoObject;
    onlineInfoObject.insert(s_bootDateTimeKey,onlineInfo.bootDateTime);
    onlineInfoObject.insert(s_shutdownDateTimeKey,onlineInfo.shutdownDateTime);
    onlineInfoObject.insert(s_lastRunDateTimeIntervalKey,onlineInfo.lastRunDateTimeInterval);
    onlineInfoObject.insert(s_lastShutdownDateTimeIntervalKey,onlineInfo.lastShutdownDateTimeInterval);
    onlineInfoObject.insert(s_lastRunDateTimeQuantumKey,onlineInfo.lastRunDateTimeQuantum);
    onlineInfoObject.insert(s_lastShutdownDateTimeQuantumKey,onlineInfo.lastShutdownDateTimeQuantum);
    onlineInfoObject.insert(KylinDaqCommon::gs_createTimeStampKey
        ,QDateTime::currentDateTime().toTimeZone(QTimeZone(KylinDaqCommon::gs_timezone)).toString(KylinDaqCommon::gs_timeFormat));
    return onlineInfoObject;
}

QJsonObject OnlineInfoModule::sourceInfo()
{
    QJsonObject sourceObj;
    QCryptographicHash cryHash(QCryptographicHash::Algorithm::Sha256);
    QString tidValidString = QString(s_packageNemeValue)+QString(s_messageTypeValue);
    cryHash.addData(tidValidString.toUtf8());
    QString nowTid(cryHash.result().toBase64());
    sourceObj.insert(KylinDaqCommon::gs_packageNameKey,s_packageNemeValue);
    sourceObj.insert(KylinDaqCommon::gs_messageTypeKey,s_messageTypeValue);
    sourceObj.insert(KylinDaqCommon::gs_tidKey,nowTid);
    return sourceObj;
}

void OnlineInfoModule::updateProperty(const QJsonObject &conf, const QJsonObject &defaultConf)
{
    qlonglong value;
    KylinDaqCommon::setValidConfInt(conf, defaultConf, moduleName(), s_pollingIntervalKey, value);
    setPollingInterval(value);
    KylinDaqCommon::setValidConfInt(conf, defaultConf, moduleName(), s_uploadTypeKey, value);
    setUploadType(value);

}

void OnlineInfoModule::createStateMessageInfo(QJsonObject &packageInfoJsonObj, QJsonObject &uploadedMessageJsonObj, KylinDaqCommon::UploadStateMessageInfo_t &uploadStateMessageInfo)
{
    QJsonObject tidsJsonObj;
    QJsonObject createTimeStampJsonObj;
    QJsonObject tempPackageInfoJsonObj;

    if (packageInfoJsonObj.contains(KylinDaqCommon::gs_packageNameKey))
    {
        tempPackageInfoJsonObj[KylinDaqCommon::gs_packageNameKey] = packageInfoJsonObj[KylinDaqCommon::gs_packageNameKey];
    }
    if (packageInfoJsonObj.contains(KylinDaqCommon::gs_messageTypeKey))
    {
        tempPackageInfoJsonObj[KylinDaqCommon::gs_messageTypeKey] = packageInfoJsonObj[KylinDaqCommon::gs_messageTypeKey];
    }
    if (packageInfoJsonObj.contains(KylinDaqCommon::gs_tidKey))
    {
        tidsJsonObj[KylinDaqCommon::gs_tidKey] = packageInfoJsonObj[KylinDaqCommon::gs_tidKey];
    }
    if (packageInfoJsonObj.contains(KylinDaqCommon::gs_parentTidKey))
    {
        tidsJsonObj[KylinDaqCommon::gs_parentTidKey] = packageInfoJsonObj[KylinDaqCommon::gs_parentTidKey];
    }
    if (uploadedMessageJsonObj.contains(KylinDaqCommon::gs_createTimeStampKey)) {
        createTimeStampJsonObj[KylinDaqCommon::gs_createTimeStampKey] = uploadedMessageJsonObj[KylinDaqCommon::gs_createTimeStampKey];
        uploadedMessageJsonObj.remove(KylinDaqCommon::gs_createTimeStampKey);
    }
    uploadStateMessageInfo.packageInfo = QString(QJsonDocument(tempPackageInfoJsonObj).toJson(QJsonDocument::Compact));
    uploadStateMessageInfo.uploadedMessage = QString(QJsonDocument(uploadedMessageJsonObj).toJson(QJsonDocument::Compact));
    uploadStateMessageInfo.tids = QString(QJsonDocument(tidsJsonObj).toJson(QJsonDocument::Compact));
    uploadStateMessageInfo.createTimeStamp = QString(QJsonDocument(createTimeStampJsonObj).toJson(QJsonDocument::Compact));

}

QString OnlineInfoModule::createMessage(KylinDaqCommon::UploadStateMessageInfo_t &uploadStateMessageInfo)
{
    QJsonObject uploadMessageInfoJsonObj;
    QJsonObject sourceJsonObj = QJsonDocument::fromJson(uploadStateMessageInfo.packageInfo.toUtf8()).object();;
    QJsonObject infoJsonObj = QJsonDocument::fromJson(uploadStateMessageInfo.uploadedMessage.toUtf8()).object();;
    QJsonObject tidsJsonObj = QJsonDocument::fromJson(uploadStateMessageInfo.tids.toUtf8()).object();
    QJsonObject createTimesStampJsonObj = QJsonDocument::fromJson(uploadStateMessageInfo.createTimeStamp.toUtf8()).object();
    if (tidsJsonObj.contains(KylinDaqCommon::gs_tidKey))
    {
        sourceJsonObj[KylinDaqCommon::gs_tidKey] = tidsJsonObj[KylinDaqCommon::gs_tidKey];
    }
    if (tidsJsonObj.contains(KylinDaqCommon::gs_parentTidKey))
    {
        sourceJsonObj[KylinDaqCommon::gs_parentTidKey] = tidsJsonObj[KylinDaqCommon::gs_parentTidKey];
    }
    if (createTimesStampJsonObj.contains(KylinDaqCommon::gs_createTimeStampKey)) {
        infoJsonObj[KylinDaqCommon::gs_createTimeStampKey] = createTimesStampJsonObj[KylinDaqCommon::gs_createTimeStampKey];
    }
    //  插入版本和类型信息
    sourceJsonObj.insert(KylinDaqCommon::gs_sourceVersionKey,KylinDaqCommon::gs_sourceVersionValue);
    uploadMessageInfoJsonObj.insert(KylinDaqCommon::gs_informationKey, infoJsonObj);
    uploadMessageInfoJsonObj.insert(KylinDaqCommon::gs_sourceKey, sourceJsonObj);
    // qDebug() << tempJsonObj;
    //  构造发送数据
    return QString(QJsonDocument(uploadMessageInfoJsonObj).toJson(QJsonDocument::Compact));

}

void OnlineInfoModule::saveStateMessageInfo(KylinDaqCommon::UploadStateMessageInfo_t &uploadStateMessageInfo,KylinDaqCommon::UploadStateMessageInfo_t &oldUploadStateMessageInfo,const KylinDaqCommon::UploadTypeEnum &uploadType)
{
    qInfo()<<"OnlineInfoModule::saveStateMessageInfo: uploadType="<<uploadType;
    if (uploadType == KylinDaqCommon::UploadTypeEnum::StateAndTwoMessage && !oldUploadStateMessageInfo.uploadedMessage.isEmpty())
    {
        saveInfo(createMessage(oldUploadStateMessageInfo));
        qInfo()<<"OnlineInfoModule::saveStateMessageInfo: save info";
    }
    if (uploadType == KylinDaqCommon::UploadTypeEnum::StateAndTwoMessage || uploadType == KylinDaqCommon::UploadTypeEnum::StateAndMessage)
    {
        saveInfo(createMessage(uploadStateMessageInfo));
        qInfo()<<"OnlineInfoModule::saveStateMessageInfo: save info";
    }
    if (oldUploadStateMessageInfo.uploadState == KylinDaqCommon::UploadStateEnum::UploadStateMessageCreated
    ||oldUploadStateMessageInfo.uploadState == KylinDaqCommon::UploadStateEnum::UploadStateMessageNull)
    {
        uploadStateMessageInfo.uploadState = KylinDaqCommon::UploadStateEnum::UploadStateMessageCreated;
    }
    else
    {
        uploadStateMessageInfo.uploadState = KylinDaqCommon::UploadStateEnum::UploadStateMessageUpdated;
    }
    if (oldUploadStateMessageInfo.uploadState == KylinDaqCommon::UploadStateEnum::UploadStateMessageNull) {
        saveStateMessage(uploadStateMessageInfo.packageInfo,
        uploadStateMessageInfo.uploadedMessage,
        uploadStateMessageInfo.tids,
        uploadStateMessageInfo.createTimeStamp,
        uploadStateMessageInfo.uploadState);
        qInfo()<<"OnlineInfoModule::saveStateMessageInfo: save state info";
    }
    else
    {
        updateStateMessage(uploadStateMessageInfo.packageInfo,
        uploadStateMessageInfo.uploadedMessage,
        uploadStateMessageInfo.tids,
        uploadStateMessageInfo.createTimeStamp,
        uploadStateMessageInfo.uploadState);
        qInfo()<<"OnlineInfoModule::saveStateMessageInfo: update state info";
    }
}
