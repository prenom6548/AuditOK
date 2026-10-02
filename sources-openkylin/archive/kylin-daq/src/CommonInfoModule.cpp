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
#include "CommonInfoModule.h"

QScopedPointer<CommonInfoModule> CommonInfoModule::ms_instance;
CommonInfoModule::CommonInfoModule(QObject *parent)
    : BaseModule(s_moduleName,parent)
    , m_timer(new QTimer(this))
{
    // connect singal solt
    connect(m_timer, &QTimer::timeout, this, &CommonInfoModule::dealInfo);
    connect(this , &CommonInfoModule::pollingIntervalChanged ,this, &CommonInfoModule::setTimePollingInterval);
}

CommonInfoModule::~CommonInfoModule()
{

}

CommonInfoModule *CommonInfoModule::instance()
{
    if (ms_instance.isNull()) {
        ms_instance.reset(new CommonInfoModule());
    }
    return ms_instance.data();
}

void CommonInfoModule::dealInfo()
{
    QString requestHeadInfo;
    QJsonObject packageInfoJsonObj;
    QJsonObject uploadedMessageJsonObj;
    KylinDaqCommon::UploadStateMessageInfo_t uploadStateMessageInfo;
    KylinDaqCommon::UploadStateMessageInfo_t oldUploadStateMessageInfo;
    QScopedPointer<CommonInfoCollection> commonInfoCollection(new CommonInfoCollection(this));
    CommonInfo commonInfo;
    commonInfoCollection->getInfo(commonInfo);
    requestHeadInfo = createRequestHeadInfo(commonInfo);
    sendInfo(requestHeadInfo);
    createStateInfo(commonInfo,packageInfoJsonObj,uploadedMessageJsonObj);
    createStateMessageInfo(packageInfoJsonObj,uploadedMessageJsonObj,uploadStateMessageInfo);
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

    // info = packageInfo(commonInfo);
    // qDebug()<<info;
    // emit sendInfo(info);
    // qInfo()<<"CommonInfoModule::dealInfo: save common info";
}

void CommonInfoModule::setTimePollingInterval(qlonglong sec)
{
    m_timer->stop();
    m_timer->setInterval(KylinDaqCommon::secToMsec(sec));
    m_timer->start();
}


QString CommonInfoModule::packageHeaderInfo(const CommonInfo &commonInfo)
{
    QJsonObject commonInfoObject;
    // commonInfoObject.insert(s_mainBoardSerialNumberKeyHeader,commonInfo.mainBoardSerialNumber);
    // commonInfoObject.insert(s_networkMacAddressKeyHeader, commonInfo.networkMacAddress);
    commonInfoObject.insert(s_processorModelKeyHeader, commonInfo.processorModel);
    commonInfoObject.insert(s_graphicsCardModelKeyHeader, commonInfo.graphicsCardModel);
    commonInfoObject.insert(s_systemVersionKeyHeader, commonInfo.systemVersion);
    commonInfoObject.insert(s_systemSerialNumberKeyHeader, commonInfo.systemSerialNumber);
    commonInfoObject.insert(s_networkTypeKeyHeader, commonInfo.networkType);
    // commonInfoObject.insert(s_hardDiskSerialNumberKeyHeader, commonInfo.hardDiskSerialNumber);
    commonInfoObject.insert(s_kernelVersionKeyHeader, commonInfo.kernelVersion);
    // commonInfoObject.insert(s_systemUuidKeyHeader, commonInfo.systemUuid);
    commonInfoObject.insert(s_systemManufacturerKeyHeader,commonInfo.systemManufacturer);
    commonInfoObject.insert(s_systemProductNameKeyHeader,commonInfo.systemProductName);
    commonInfoObject.insert(s_machineIdKeyHeader, commonInfo.machineId);
    // commonInfoObject.insert(s_registerNumberHeader, commonInfo.registerNumber);
    commonInfoObject.insert(s_dpkgVersionKeyHeader, commonInfo.dpkgVersion);
    // commonInfoObject.insert(s_kysecVersionKeyHeader, commonInfo.kysecVersion);
    // commonInfoObject.insert(s_kysecAuthVersionKeyHeader, commonInfo.kysecAuthVersion);
    // commonInfoObject.insert(s_kysecDaemonVersionKeyHeader, commonInfo.kysecDaemonVersion);
    // commonInfoObject.insert(s_kysecUtilsVersionKeyHeader, commonInfo.kysecUtilsVersion);
    // commonInfoObject.insert(s_libkysecVersionKeyHeader, commonInfo.libkysecVersion);
    commonInfoObject.insert(s_osVersionKeyHeader,commonInfo.osVersion);
    commonInfoObject.insert(s_osReleaseIdKeyHeader,commonInfo.osReleaseId);
    commonInfoObject.insert(s_osBuildInfoKeyHeader,commonInfo.osBuildInfo);
    commonInfoObject.insert(s_systemArchitectureKeyHeader,commonInfo.systemArchitecture);
    commonInfoObject.insert(s_systemLanguageKeyHeader,commonInfo.systemLanguage);
    return QJsonDocument(commonInfoObject).toJson(QJsonDocument::JsonFormat::Compact);

}

QString CommonInfoModule::packageInfo(const CommonInfo &commonInfo)
{
    QJsonObject commonInfoObject;
    // commonInfoObject.insert(s_mainBoardSerialNumberKey,commonInfo.mainBoardSerialNumber);
    // commonInfoObject.insert(s_networkMacAddressKey, commonInfo.networkMacAddress);
    commonInfoObject.insert(s_processorModelKey, commonInfo.processorModel);
    commonInfoObject.insert(s_graphicsCardModelKey, commonInfo.graphicsCardModel);
    commonInfoObject.insert(s_systemVersionKey, commonInfo.systemVersion);
    commonInfoObject.insert(s_systemSerialNumberKey, commonInfo.systemSerialNumber);
    commonInfoObject.insert(s_networkTypeKey, commonInfo.networkType);
    // commonInfoObject.insert(s_hardDiskSerialNumberKey, commonInfo.hardDiskSerialNumber);
    commonInfoObject.insert(s_kernelVersionKey, commonInfo.kernelVersion);
    // commonInfoObject.insert(s_systemUuidKey, commonInfo.systemUuid);
    commonInfoObject.insert(s_systemManufacturerKey,commonInfo.systemManufacturer);
    commonInfoObject.insert(s_systemProductNameKey,commonInfo.systemProductName);
    commonInfoObject.insert(s_machineIdKey, commonInfo.machineId);
    // commonInfoObject.insert(s_registerNumber, commonInfo.registerNumber);
    commonInfoObject.insert(s_dpkgVersionKey, commonInfo.dpkgVersion);
    // commonInfoObject.insert(s_kysecVersionKey, commonInfo.kysecVersion);
    // commonInfoObject.insert(s_kysecAuthVersionKey, commonInfo.kysecAuthVersion);
    // commonInfoObject.insert(s_kysecDaemonVersionKey, commonInfo.kysecDaemonVersion);
    // commonInfoObject.insert(s_kysecUtilsVersionKey, commonInfo.kysecUtilsVersion);
    // commonInfoObject.insert(s_libkysecVersionKey, commonInfo.libkysecVersion);
    commonInfoObject.insert(s_osVersionKey,commonInfo.osVersion);
    commonInfoObject.insert(s_osReleaseIdKey,commonInfo.osReleaseId);
    commonInfoObject.insert(s_osBuildInfoKey,commonInfo.osBuildInfo);
    commonInfoObject.insert(s_osTreeVersionKey,commonInfo.osTreeVersion);
    commonInfoObject.insert(s_osPatchVersionKey,commonInfo.osPatchVersion);
    commonInfoObject.insert(s_osSerialNumberKey,commonInfo.osSerialNumber);
    commonInfoObject.insert(s_systemArchitectureKey,commonInfo.systemArchitecture);
    commonInfoObject.insert(s_systemLanguageKey,commonInfo.systemLanguage);
    return QJsonDocument(commonInfoObject).toJson(QJsonDocument::JsonFormat::Compact);

}

void CommonInfoModule::printInfo(const CommonInfo &commonInfo)
{
    qDebug()<<"mainboardSerialNumber:"<<commonInfo.mainBoardSerialNumber;
    qDebug()<<"systemVersion:"<<commonInfo.systemVersion;
    qDebug()<<"systemSerialNumber:"<<commonInfo.systemSerialNumber;
    qDebug()<<"systemUuid:"<<commonInfo.systemUuid;
    qDebug()<<"kernelVersion:"<<commonInfo.kernelVersion;
    qDebug()<<"processorModel"<<commonInfo.processorModel;
    qDebug()<<"graphicsCardModel"<<commonInfo.graphicsCardModel;
    qDebug()<<"hardDiskSerialNumber"<<commonInfo.hardDiskSerialNumber;
    qDebug()<<"networkMacAddress"<<commonInfo.networkMacAddress;
    qDebug()<<"networkType"<<commonInfo.networkType;
    qDebug()<<"systemManufacturer:"<<commonInfo.systemManufacturer;
    qDebug()<<"systemProductName:"<<commonInfo.systemProductName;
    qDebug()<<"registerNumber:"<<commonInfo.registerNumber;
    qDebug()<<"systemArchitecture:"<<commonInfo.systemArchitecture;
    qDebug()<<"systemLanguage:"<<commonInfo.systemLanguage;
}

void CommonInfoModule::updateProperty(const QJsonObject &conf,const QJsonObject &defaultConf)
{
    qlonglong value;
    KylinDaqCommon::setValidConfInt(conf, defaultConf, moduleName(), s_pollingIntervalKey, value);
    setPollingInterval(value);
    KylinDaqCommon::setValidConfInt(conf, defaultConf, moduleName(), s_uploadTypeKey, value);
    setUploadType(value);
}

void CommonInfoModule::createStateMessageInfo(QJsonObject &packageInfoJsonObj, QJsonObject &uploadedMessageJsonObj, KylinDaqCommon::UploadStateMessageInfo_t &uploadStateMessageInfo)
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

QString CommonInfoModule::createMessage(KylinDaqCommon::UploadStateMessageInfo_t &uploadStateMessageInfo)
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

void CommonInfoModule::saveStateMessageInfo(KylinDaqCommon::UploadStateMessageInfo_t &uploadStateMessageInfo,KylinDaqCommon::UploadStateMessageInfo_t &oldUploadStateMessageInfo,const KylinDaqCommon::UploadTypeEnum &uploadType)
{
    qInfo()<<"CommonInfoModule::saveStateMessageInfo: uploadType="<<uploadType;
    if (uploadType == KylinDaqCommon::UploadTypeEnum::StateAndTwoMessage && !oldUploadStateMessageInfo.uploadedMessage.isEmpty())
    {
        saveInfo(createMessage(oldUploadStateMessageInfo));
        qInfo()<<"CommonInfoModule::saveStateMessageInfo: save info";
    }
    if (uploadType == KylinDaqCommon::UploadTypeEnum::StateAndTwoMessage || uploadType == KylinDaqCommon::UploadTypeEnum::StateAndMessage)
    {
        saveInfo(createMessage(uploadStateMessageInfo));
        qInfo()<<"CommonInfoModule::saveStateMessageInfo: save info";
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
        qInfo()<<"CommonInfoModule::saveStateMessageInfo: save state info";
    }
    else
    {
        updateStateMessage(uploadStateMessageInfo.packageInfo,
        uploadStateMessageInfo.uploadedMessage,
        uploadStateMessageInfo.tids,
        uploadStateMessageInfo.createTimeStamp,
        uploadStateMessageInfo.uploadState);
        qInfo()<<"CommonInfoModule::saveStateMessageInfo: update state info";
    }
}



QString CommonInfoModule::createRequestHeadInfo(const CommonInfo &commonInfo)
{
    QCryptographicHash cryHash(QCryptographicHash::Algorithm::Sha256);
    QJsonObject commonInfoJsonObj;
    cryHash.addData(packageHeaderInfo(commonInfo).toUtf8());
    commonInfoJsonObj.insert(s_commonInfoSha256KeyHeader,QString(cryHash.result().toBase64()));
    commonInfoJsonObj.insert(s_machineIdKeyHeader,commonInfo.machineId);
    commonInfoJsonObj.insert(s_systemSerialNumberKeyHeader,commonInfo.systemSerialNumber);
    commonInfoJsonObj.insert(s_registerNumberHeader,commonInfo.registerNumber);
    commonInfoJsonObj.insert(s_osSerialNumberKeyHeader,commonInfo.osSerialNumber);
    return QJsonDocument(commonInfoJsonObj).toJson(QJsonDocument::JsonFormat::Compact);
}

void CommonInfoModule::createStateInfo(const CommonInfo &commonInfo,QJsonObject &packageInfoJsonObj, QJsonObject &uploadedMessageJsonObj)
{
    QCryptographicHash cryHash(QCryptographicHash::Algorithm::Sha256);
    QString tidValidString = QString(s_commonInfoPackageNameValue)+QString(s_commonInfoMessageTypeValue);
    cryHash.addData(tidValidString.toUtf8());
    QString nowTid(cryHash.result().toBase64());
    packageInfoJsonObj.insert(KylinDaqCommon::gs_packageNameKey,s_commonInfoPackageNameValue);
    packageInfoJsonObj.insert(KylinDaqCommon::gs_messageTypeKey,s_commonInfoMessageTypeValue);
    packageInfoJsonObj.insert(KylinDaqCommon::gs_tidKey,nowTid);
    uploadedMessageJsonObj = QJsonDocument::fromJson(packageInfo(commonInfo).toUtf8()).object();
    uploadedMessageJsonObj.insert(KylinDaqCommon::gs_createTimeStampKey,QDateTime::currentDateTime().toTimeZone(QTimeZone(KylinDaqCommon::gs_timezone)).toString(KylinDaqCommon::gs_timeFormat));
}
