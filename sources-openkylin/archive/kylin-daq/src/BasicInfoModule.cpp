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
#include "BasicInfoModule.h"

QScopedPointer<BasicInfoModule> BasicInfoModule::ms_instance;
BasicInfoModule::BasicInfoModule(QObject *parent)
    :BaseModule(s_moduleName,parent)
    ,m_timer(new QTimer(this))
{
    connect(m_timer, &QTimer::timeout, this, &BasicInfoModule::dealInfo);
    connect(this , &BasicInfoModule::pollingIntervalChanged ,this, &BasicInfoModule::setTimerPollingInterval);
}

BasicInfoModule::~BasicInfoModule()
{

}

BasicInfoModule *BasicInfoModule::instance()
{
    if (ms_instance.isNull()) {
        ms_instance.reset(new BasicInfoModule());
    }
    return ms_instance.data();
}

void BasicInfoModule::dealInfo()
{
    BasicInfo basicInfo;
    QScopedPointer<BasicInfoCollection> basicInfoCollection(new BasicInfoCollection(this));
    basicInfoCollection->getInfo(basicInfo);
    QJsonObject sourceObj = sourceInfo();
    QJsonObject informationObj = packageBasicInfo(basicInfo);
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
    // QJsonObject sendedInfoObj;
    // QString sendedInfoStr;
    // sendedInfoObj.insert(KylinDaqCommon::gs_informationKey,informationObj);
    // sendedInfoObj.insert(KylinDaqCommon::gs_sourceKey,sourceObj);
    // sendedInfoStr = QJsonDocument(sendedInfoObj).toJson(QJsonDocument::Compact);
    // emit  sendInfo(sendedInfoStr);
}

void BasicInfoModule::setTimerPollingInterval(qlonglong sec)
{
    m_timer->stop();
    m_timer->setInterval(KylinDaqCommon::secToMsec(sec));
    m_timer->start();
}

QJsonObject BasicInfoModule::packageBasicInfo(const BasicInfo &basicInfo)
{
    QJsonObject basicInfoObject;
    basicInfoObject.insert(s_kernelVersionKey,basicInfo.kernelVersion);
    basicInfoObject.insert(s_gccVersionKey,basicInfo.gccVersion);
    basicInfoObject.insert(s_glibcVersiKey,basicInfo.glibcVersi);
    basicInfoObject.insert(s_gdbVersionKey,basicInfo.gdbVersion);
    basicInfoObject.insert(s_qt5VersionKey,basicInfo.qt5Version);
    basicInfoObject.insert(s_qt4VersionKey,basicInfo.qt4Version);
    basicInfoObject.insert(s_python3VersionKey,basicInfo.python3Version);
    basicInfoObject.insert(s_python2VersionKey,basicInfo.python2Version);
    basicInfoObject.insert(s_javaVersionKey,basicInfo.javaVersion);
    basicInfoObject.insert(s_grubVersionKey,basicInfo.grubVersion);
    basicInfoObject.insert(s_cupsVersionKey,basicInfo.cupsVersion);
    basicInfoObject.insert(s_libjbigVersionKey,basicInfo.libjbigVersion);
    basicInfoObject.insert(s_libpngVersionKey,basicInfo.libpngVersion);
    basicInfoObject.insert(s_libjpegKey,basicInfo.libjpeg);
    basicInfoObject.insert(KylinDaqCommon::gs_createTimeStampKey
        ,QDateTime::currentDateTime().toTimeZone(QTimeZone(KylinDaqCommon::gs_timezone)).toString(KylinDaqCommon::gs_timeFormat));
    basicInfoObject.insert(s_kylinDaqVersionKey,basicInfo.kylinDaqVersion);
    printInfo(basicInfo);
    return basicInfoObject;
}

QJsonObject BasicInfoModule::sourceInfo()
{
    QJsonObject sourceObj;
    QCryptographicHash cryHash(QCryptographicHash::Algorithm::Sha256);
    QString tidValidString = QString(s_packageNemeValue)+QString(s_messageTypeValue);
    cryHash.addData(tidValidString.toUtf8());
    QString nowTid(cryHash.result().toBase64());
    sourceObj.insert(KylinDaqCommon::gs_packageNameKey,s_packageNemeValue);
    sourceObj.insert(KylinDaqCommon::gs_messageTypeKey,s_messageTypeValue);
    // sourceObj.insert(KylinDaqCommon::gs_sourceVersionKey,KylinDaqCommon::gs_sourceVersionValue);
    sourceObj.insert(KylinDaqCommon::gs_tidKey,nowTid);
    return sourceObj;
}

void BasicInfoModule::updateProperty(const QJsonObject &conf, const QJsonObject &defaultConf)
{
    qlonglong value;
    KylinDaqCommon::setValidConfInt(conf, defaultConf, moduleName(), s_pollingIntervalKey, value);
    setPollingInterval(value);
    KylinDaqCommon::setValidConfInt(conf, defaultConf, moduleName(), s_uploadTypeKey, value);
    setUploadType(value);

}

void BasicInfoModule::printInfo(const BasicInfo &basicInfo)
{
    qDebug() << s_kernelVersionKey << basicInfo.kernelVersion;
    qDebug() << s_gccVersionKey << basicInfo.gccVersion;
    qDebug() << s_glibcVersiKey << basicInfo.glibcVersi;
    qDebug() << s_gdbVersionKey << basicInfo.gdbVersion;
    qDebug() << s_qt5VersionKey << basicInfo.qt5Version;
    qDebug() << s_qt4VersionKey << basicInfo.qt4Version;
    qDebug() << s_python3VersionKey << basicInfo.python3Version;
    qDebug() << s_python2VersionKey << basicInfo.python2Version;
    qDebug() << s_javaVersionKey << basicInfo.javaVersion;
    qDebug() << s_grubVersionKey << basicInfo.grubVersion;
    qDebug() << s_cupsVersionKey << basicInfo.cupsVersion;
    qDebug() << s_libjbigVersionKey << basicInfo.libjbigVersion;
    qDebug() << s_libpngVersionKey << basicInfo.libpngVersion;
    qDebug() << s_libjpegKey << basicInfo.libjpeg;
    qDebug() << s_kylinDaqVersionKey << basicInfo.kylinDaqVersion;
}

void BasicInfoModule::createStateMessageInfo(QJsonObject &packageInfoJsonObj, QJsonObject &uploadedMessageJsonObj, KylinDaqCommon::UploadStateMessageInfo_t &uploadStateMessageInfo)
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

QString BasicInfoModule::createMessage(KylinDaqCommon::UploadStateMessageInfo_t &uploadStateMessageInfo)
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

void BasicInfoModule::saveStateMessageInfo(KylinDaqCommon::UploadStateMessageInfo_t &uploadStateMessageInfo,KylinDaqCommon::UploadStateMessageInfo_t &oldUploadStateMessageInfo,const KylinDaqCommon::UploadTypeEnum &uploadType)
{
    qInfo()<<"BasicInfoModule::saveStateMessageInfo: uploadType="<<uploadType;
    if (uploadType == KylinDaqCommon::UploadTypeEnum::StateAndTwoMessage && !oldUploadStateMessageInfo.uploadedMessage.isEmpty())
    {
        saveInfo(createMessage(oldUploadStateMessageInfo));
        qInfo()<<"BasicInfoModule::saveStateMessageInfo: save info";
    }
    if (uploadType == KylinDaqCommon::UploadTypeEnum::StateAndTwoMessage || uploadType == KylinDaqCommon::UploadTypeEnum::StateAndMessage)
    {
        saveInfo(createMessage(uploadStateMessageInfo));
        qInfo()<<"BasicInfoModule::saveStateMessageInfo: save info";
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
        qInfo()<<"BasicInfoModule::saveStateMessageInfo: save state info";
    }
    else
    {
        updateStateMessage(uploadStateMessageInfo.packageInfo,
        uploadStateMessageInfo.uploadedMessage,
        uploadStateMessageInfo.tids,
        uploadStateMessageInfo.createTimeStamp,
        uploadStateMessageInfo.uploadState);
        qInfo()<<"BasicInfoModule::saveStateMessageInfo: update state info";
    }
}
