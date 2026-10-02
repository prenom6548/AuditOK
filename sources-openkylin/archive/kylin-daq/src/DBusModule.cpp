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
/*** 
 * @Author: pengcheng
 * @Date: 2021-06-24 10:27:50
 * @LastEditTime: 2021-08-24 14:41:01
 * @LastEditors: pengcheng
 * @Description: 
 * @FilePath: /kylin-daq/src/DataCollection.cpp
 */
#include "DBusModule.h"
#include <qglobal.h>
#include <qjsondocument.h>
#include <openssl/crypto.h>   // 为 OPENSSL_cleanse


// ================== 原有实现 ==================

QScopedPointer<DBusModule> DBusModule::ms_instance;

DBusModule *DBusModule::instance()
{
    if (ms_instance.isNull()) {
        ms_instance.reset(new DBusModule());
    }
    return ms_instance.data();
}

DBusModule::DBusModule(QObject *parent)
    : BaseModule(s_moduleName,parent), m_uploadState(KylinDaqCommon::UploadState::UploadEnable)
{
    if (QDBusConnection::systemBus().isConnected()) {
        QDBusConnection dBusConnection(QDBusConnection::systemBus());
        if (dBusConnection.registerService(s_serverName)) {
            if (dBusConnection.registerObject(s_objectPath, s_interfaceName, this, QDBusConnection::ExportScriptableSlots | QDBusConnection::ExportScriptableSignals)) {
                initDisableCommonKey();
            } else {
                qCritical() << "module=" << moduleName() << ":" << QString("Cannot register object ")<<QDBusConnection::systemBus().lastError().message();
            }
        } else {
               qCritical() << "module=" << moduleName() << ":" << QString("Cannot register service ")<<QDBusConnection::systemBus().lastError().message();
        }
    } else {
        qCritical() << "module=" << moduleName() << ":" << QString("Cannot connect to the D-Bus system bus.");
    }
}

void DBusModule::getPackageInfo(const QString &packInfo, QString &packageName, QString &messageType)
{
    QJsonDocument packInfoJsonDoc = QJsonDocument::fromJson(packInfo.toUtf8());
    if(!packInfoJsonDoc.isObject())
    {
        return ;
    }
    // 检查是否有必备字段
    QJsonObject packInfoJsonObj = packInfoJsonDoc.object();
    if(!packInfoJsonObj.contains(KylinDaqCommon::gs_packageNameKey)||!packInfoJsonObj[KylinDaqCommon::gs_packageNameKey].isString())
    {
        return;
    }
    else
    {
        packageName = packInfoJsonObj[KylinDaqCommon::gs_packageNameKey].toString();
    }
    if(!packInfoJsonObj.contains(KylinDaqCommon::gs_messageTypeKey)||!packInfoJsonObj[KylinDaqCommon::gs_messageTypeKey].isString())
    {
        return;
    }
    else
    {
        messageType = packInfoJsonObj[KylinDaqCommon::gs_messageTypeKey].toString();
    }
    return ;
}

DBusModule::~DBusModule()
{
}

void DBusModule::initDisableCommonKey()
{
    m_disableCommonKey << "@timestamp";
    m_disableCommonKey << "_id";
    m_disableCommonKey << "_index";
    m_disableCommonKey << "_type";
    m_disableCommonKey << "createTime";
    m_disableCommonKey << "highlight";
    m_disableCommonKey << "sn";
    m_disableCommonKey << "sort";
}

QByteArray DBusModule::decrypt(QByteArray hasedValue)
{
    BIO *bio=nullptr;
    RSA *p_rsa=nullptr;
    EVP_PKEY *key=nullptr;
    EVP_PKEY_CTX *ctx=nullptr;
    unsigned char *out=nullptr;
    size_t outlen;

    // 使用拆分+XOR+拼接后的 base64 私钥，运行时构造，并在最后清理
    QByteArray privB64 = "";
    QByteArray tempPrivateKey = QByteArray::fromBase64(privB64);
    if (tempPrivateKey.isEmpty())
    {
        qWarning() << "DBusModule::decrypt: private key base64 decode failed.";
        return QByteArray();
    }

    if((bio = BIO_new_mem_buf(tempPrivateKey.data(),tempPrivateKey.size())) == NULL)
    {
        evpError();
        OPENSSL_cleanse(tempPrivateKey.data(), tempPrivateKey.size());
        OPENSSL_cleanse(privB64.data(), privB64.size());
        return QByteArray();
    }
    if((p_rsa=PEM_read_bio_RSAPrivateKey(bio,NULL,NULL,NULL))==NULL)
    {
        evpError();
        BIO_free(bio);
        OPENSSL_cleanse(tempPrivateKey.data(), tempPrivateKey.size());
        OPENSSL_cleanse(privB64.data(), privB64.size());
        return QByteArray();
    }
    key = EVP_PKEY_new();
    if (key == NULL)
    {
        evpError();
        BIO_free(bio);
        RSA_free(p_rsa);
        OPENSSL_cleanse(tempPrivateKey.data(), tempPrivateKey.size());
        OPENSSL_cleanse(privB64.data(), privB64.size());
        return QByteArray();
    }
    if(EVP_PKEY_set1_RSA(key,p_rsa)<=0)
    {
        evpError();
        BIO_free(bio);
        RSA_free(p_rsa);
        EVP_PKEY_free(key);
        OPENSSL_cleanse(tempPrivateKey.data(), tempPrivateKey.size());
        OPENSSL_cleanse(privB64.data(), privB64.size());
        return QByteArray();
    }
    ctx = EVP_PKEY_CTX_new(key,NULL);
    if(ctx == NULL)
    {
        evpError();
        BIO_free(bio);
        RSA_free(p_rsa);
        EVP_PKEY_free(key);
        OPENSSL_cleanse(tempPrivateKey.data(), tempPrivateKey.size());
        OPENSSL_cleanse(privB64.data(), privB64.size());
        return QByteArray();
    }
    if(EVP_PKEY_decrypt_init(ctx) <=0)
    {
        evpError();
        BIO_free(bio);
        RSA_free(p_rsa);
        EVP_PKEY_free(key);
        EVP_PKEY_CTX_free(ctx);
        OPENSSL_cleanse(tempPrivateKey.data(), tempPrivateKey.size());
        OPENSSL_cleanse(privB64.data(), privB64.size());
        return QByteArray();
    }
    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0)
    {
        evpError();
        BIO_free(bio);
        RSA_free(p_rsa);
        EVP_PKEY_free(key);
        EVP_PKEY_CTX_free(ctx);
        OPENSSL_cleanse(tempPrivateKey.data(), tempPrivateKey.size());
        OPENSSL_cleanse(privB64.data(), privB64.size());
        return QByteArray();
    }

    /* Determine buffer length */
    if (EVP_PKEY_decrypt(ctx, NULL, &outlen, reinterpret_cast<const unsigned char *>(hasedValue.data()), hasedValue.size()) <= 0)
    {
        evpError();
        BIO_free(bio);
        RSA_free(p_rsa);
        EVP_PKEY_free(key);
        EVP_PKEY_CTX_free(ctx);
        OPENSSL_cleanse(tempPrivateKey.data(), tempPrivateKey.size());
        OPENSSL_cleanse(privB64.data(), privB64.size());
        return QByteArray();
    }

    out = static_cast<unsigned char *> (OPENSSL_malloc(outlen));

    if (!out)
    {
        evpError();
        BIO_free(bio);
        RSA_free(p_rsa);
        EVP_PKEY_free(key);
        EVP_PKEY_CTX_free(ctx);
        OPENSSL_cleanse(tempPrivateKey.data(), tempPrivateKey.size());
        OPENSSL_cleanse(privB64.data(), privB64.size());
        return QByteArray();
    }
    /* malloc failure */

    if (EVP_PKEY_decrypt(ctx, out, &outlen, reinterpret_cast<const unsigned char *>(hasedValue.data()), hasedValue.size()) <= 0)
    {
        evpError();
        BIO_free(bio);
        RSA_free(p_rsa);
        EVP_PKEY_free(key);
        EVP_PKEY_CTX_free(ctx);
        OPENSSL_free(out);
        OPENSSL_cleanse(tempPrivateKey.data(), tempPrivateKey.size());
        OPENSSL_cleanse(privB64.data(), privB64.size());
        return QByteArray();
    }
    QByteArray retByteArray(reinterpret_cast<const char *> (out),outlen);
    BIO_free(bio);
    RSA_free(p_rsa);
    EVP_PKEY_free(key);
    EVP_PKEY_CTX_free(ctx);
    OPENSSL_free(out);

    OPENSSL_cleanse(tempPrivateKey.data(), tempPrivateKey.size());
    OPENSSL_cleanse(privB64.data(), privB64.size());

    return retByteArray;
}

void DBusModule::evpError(void)
{
    int n  = ERR_get_error();
    char szErr[512];
    char errStr[1024];
    ERR_error_string( n, szErr );
    snprintf(errStr,sizeof(errStr),"error code = %d,code string = %s",n,szErr);
    qWarning() << errStr;
}

DBusModule::UploadMessageReturnState DBusModule::checkHashedValue(const QString &information, const QString &hasedValue)
{
    qDebug()<<__FUNCTION__<<"hasedValue="<<hasedValue;
    QByteArray enHasedValue = QByteArray::fromBase64(hasedValue.toUtf8());
    QByteArray deHasedValue = decrypt(enHasedValue);
    if(deHasedValue.isEmpty())
    {
        qDebug()<<__FUNCTION__<<"decrypt haseValue failed";
        return UploadMessageReturnState::InvalidUploadedMessageSha256Decryption;
    }
    QCryptographicHash cryHash(QCryptographicHash::Algorithm::Sha256);
    cryHash.addData(information.toUtf8());
    QByteArray nowHasedValue(cryHash.result());
    if(deHasedValue != nowHasedValue)
    {
        qDebug()<<__FUNCTION__<<"haseValue != uploadedMessage hase value";
        return UploadMessageReturnState::InvalidUploadedMessageSha256;
    }
    return UploadMessageReturnState::OK;
}

DBusModule::UploadMessageReturnState DBusModule::checkPackageInfo(const QString &packInfo)
{
    UploadMessageReturnState retState = UploadMessageReturnState::OK;
    // 检查是否为json字符串
    QJsonDocument packInfoJsonDoc = QJsonDocument::fromJson(packInfo.toUtf8());
    qDebug()<<__FUNCTION__<<"packageInfo="<<packInfo;
    if(!packInfoJsonDoc.isObject())
    {
        qDebug()<<__FUNCTION__<<"packageInfo is not json object";
        retState =UploadMessageReturnState::InvalidArgumentFormat;
        return retState;
    }
    // 检查是否有必备字段
    QJsonObject packInfoJsonObj = packInfoJsonDoc.object();
    if(!packInfoJsonObj.contains(KylinDaqCommon::gs_packageNameKey)||!packInfoJsonObj[KylinDaqCommon::gs_packageNameKey].isString())
    {
        qDebug()<<__FUNCTION__<<"packageInfo is not contain packageName key";
        retState =UploadMessageReturnState::InvalidArgumentFormat;
        return retState;
    }
    if(!packInfoJsonObj.contains(KylinDaqCommon::gs_messageTypeKey)||!packInfoJsonObj[KylinDaqCommon::gs_messageTypeKey].isString())
    {
        qDebug()<<__FUNCTION__<<"packageInfo is not contain messageType key";
        retState =UploadMessageReturnState::InvalidArgumentFormat;
        return retState;
    }
    //  检查禁止字段
    retState = checkPackageInfoInvalidKey(packInfoJsonObj);
    if(retState != UploadMessageReturnState::OK)
    {
        qDebug()<<__FUNCTION__<<"packageInfo is contain disable key";
        return retState;
    }
    //	检查tid
    QString validString = packInfoJsonObj[KylinDaqCommon::gs_packageNameKey].toString() + packInfoJsonObj[KylinDaqCommon::gs_messageTypeKey].toString();
    QCryptographicHash cryHash(QCryptographicHash::Algorithm::Sha256);
    cryHash.addData(validString.toUtf8());
    QString nowTid(cryHash.result().toBase64());
    QString tid;
    if (packInfoJsonObj.contains(KylinDaqCommon::gs_tidKey) && packInfoJsonObj[KylinDaqCommon::gs_tidKey].isString())
    {
        tid = packInfoJsonObj[KylinDaqCommon::gs_tidKey].toString();
    }

    if (nowTid != tid)
    {
        qDebug()<<__FUNCTION__<<"nowTid:"<<nowTid;
        qDebug()<<__FUNCTION__<<"tid:"<<tid;
        return UploadMessageReturnState::InvalidTid;
    }
    return UploadMessageReturnState::OK;
}

DBusModule::UploadMessageReturnState DBusModule::checkPackageInfoInvalidKey(const QJsonObject &packageInfoJsonObj)
{
    foreach (QString tempStr, m_disableCommonKey) {
        if(packageInfoJsonObj.contains(tempStr))
        {
            qDebug()<<__FUNCTION__<<"packageInfo contain disable key:"<<tempStr;
            return UploadMessageReturnState::DisableKey;
        }
    }
    return UploadMessageReturnState::OK;
}

DBusModule::UploadMessageReturnState DBusModule::checkUploadedMessage(const QString &uploadedMessage)
{
    UploadMessageReturnState retState = UploadMessageReturnState::OK;
    qDebug()<<__FUNCTION__<<"uploadedMessage="<<uploadedMessage;
    QJsonDocument uploadedMessageJsonDoc = QJsonDocument::fromJson(uploadedMessage.toUtf8());
    // 检测是否为json对象
    if(!uploadedMessageJsonDoc.isObject())
    {
        qDebug()<<__FUNCTION__<<"uploadedMessage is not json object";
        retState = UploadMessageReturnState::InvalidArgumentFormat;
        return retState;
    }
    // 检测时间戳状态
    retState = checkUploadedMessageTimeStampState(uploadedMessageJsonDoc.object());
    if(retState != UploadMessageReturnState::OK)
    {
        return retState;
    }
    // 检测是否包含无效字段
    retState = checkUploadedMessageInvalidKey(uploadedMessageJsonDoc.object());
    return retState;
}

DBusModule::UploadMessageReturnState DBusModule::checkUploadedMessageInvalidKey(const QJsonObject &uploadedMessageJsonObj)
{
    if(uploadedMessageJsonObj.contains(KylinDaqCommon::gs_messageTypeKey)
       || uploadedMessageJsonObj.contains(KylinDaqCommon::gs_packageNameKey)
       || uploadedMessageJsonObj.contains(KylinDaqCommon::gs_tidKey)
       || uploadedMessageJsonObj.contains(KylinDaqCommon::gs_parentTidKey)
      )
    {
        qDebug()<<__FUNCTION__<<"uploadMessage contain invalid key";
        return UploadMessageReturnState::DisableKey;
    }
    foreach (QString tempStr, m_disableCommonKey) {
        if(uploadedMessageJsonObj.contains(tempStr))
        {
            qDebug()<<__FUNCTION__<<"uploadMessage contain invalid key:"<<tempStr;
            return UploadMessageReturnState::DisableKey;
        }
    }
    return UploadMessageReturnState::OK;

}

DBusModule::UploadMessageReturnState DBusModule::checkUploadedMessageTimeStampState(const QJsonObject &uploadedMessageJsonObj)
{

    if(!uploadedMessageJsonObj.contains(KylinDaqCommon::gs_createTimeStampKey) || !uploadedMessageJsonObj[KylinDaqCommon::gs_createTimeStampKey].isString())
    {
        qDebug()<<__FUNCTION__<<"createTimeStamp invalid";
        return UploadMessageReturnState::InvalidCreateTimeStamp;
    }
    QString tempDateTimeStr = uploadedMessageJsonObj[KylinDaqCommon::gs_createTimeStampKey].toString();
    QDateTime tempDateTime = QDateTime::fromString(tempDateTimeStr, KylinDaqCommon::gs_timeFormat);
    if(!tempDateTime.isValid())
    {
        qDebug()<<__FUNCTION__<<"createTimeStamp invalid time stamp:"<<uploadedMessageJsonObj[KylinDaqCommon::gs_createTimeStampKey].toString();
        return UploadMessageReturnState::InvalidCreateTimeStamp;
    }
    return UploadMessageReturnState::OK;
}

QString DBusModule::createPackInfoId(const QString &packInfo)
{
    QJsonDocument packInfoJsonDoc = QJsonDocument::fromJson(packInfo.toUtf8());
    QJsonObject packInfoJsonObj = packInfoJsonDoc.object();
    QString validString = packInfoJsonObj[KylinDaqCommon::gs_packageNameKey].toString() + packInfoJsonObj[KylinDaqCommon::gs_messageTypeKey].toString();
    QCryptographicHash cryHash(QCryptographicHash::Algorithm::Sha256);
    cryHash.addData(validString.toUtf8());
    QJsonObject tidsJsonObj;
    tidsJsonObj[KylinDaqCommon::gs_tidKey] = QString(cryHash.result().toBase64());
    return QString(QJsonDocument(tidsJsonObj).toJson(QJsonDocument::Compact));
}

QString DBusModule::getPackageInfoTid(const QString &packInfo)
{
    QJsonDocument packInfoJsonDoc = QJsonDocument::fromJson(packInfo.toUtf8());
    QJsonObject packInfoJsonObj = packInfoJsonDoc.object();
    if (packInfoJsonObj.contains(KylinDaqCommon::gs_tidKey) && packInfoJsonObj[KylinDaqCommon::gs_tidKey].isString())
    {
        return packInfoJsonObj[KylinDaqCommon::gs_tidKey].toString();
    }
    return "";

}

QString DBusModule::fillSource(const QString &packageInfo, const QString &tid, const QString &newTid)
{
    QJsonDocument packInfoJsonDoc = QJsonDocument::fromJson(packageInfo.toUtf8());
    QJsonObject packInfoJsonObj = packInfoJsonDoc.object();
    if (newTid == "")
    {
        return packageInfo;
    }
    else
    {
        packInfoJsonObj[KylinDaqCommon::gs_tidKey] = newTid;
        packInfoJsonObj[KylinDaqCommon::gs_parentTidKey] = tid;
        return QString(QJsonDocument(packInfoJsonObj).toJson(QJsonDocument::JsonFormat::Compact));
    }
}


QString DBusModule::fillValidInfo(const QString &source, const QString &uploadedMessage)
{
    QJsonDocument infoJsonDoc = QJsonDocument::fromJson(uploadedMessage.toUtf8());
    QJsonDocument sourceDoc = QJsonDocument::fromJson(source.toUtf8());
    QJsonObject sourceObject = sourceDoc.object();
    QJsonObject infoObj = infoJsonDoc.object();
    QJsonObject tempJsonObj;
    //  插入版本和类型信息
    sourceObject.insert(KylinDaqCommon::gs_sourceVersionKey,KylinDaqCommon::gs_sourceVersionValue);
    tempJsonObj.insert(KylinDaqCommon::gs_informationKey, infoObj);
    tempJsonObj.insert(KylinDaqCommon::gs_sourceKey, sourceObject);
    // qDebug() << tempJsonObj;
    //  构造发送数据
    return QString(QJsonDocument(tempJsonObj).toJson(QJsonDocument::Compact));
}


int DBusModule::GetUploadState()
{
    qInfo()<<"DBusModule::GetUploadState: uploadState="<<m_uploadState;
    return static_cast<int>(m_uploadState);
}

int DBusModule::SetUploadState(int setState)
{

    if(setState == KylinDaqCommon::UploadState::UploadDisable || setState == KylinDaqCommon::UploadState::UploadEnable)
    {
        qInfo()<<"DBusModule::SetUploadState: uploadState="<<setState;
        KylinDaqCommon::UploadState uploadState = KylinDaqCommon::UploadState(setState);
        setUploadState(uploadState);
        if (m_uploadState == uploadState) {
            emit UploadStateChanged(setState);
        }
    }
    else
    {
        qWarning()<<"DBusModule::SetUploadState: uploadState="<<setState<<" is invalid";
       return SetState::inValid;
    }
    return SetState::Valid;
}


int DBusModule::UploadEventMessage(const QString &uploadedMessage, const QString &uploadedMessageSha256)
{

//    qDebug() << "调用事件接口，" << uploadedMessage;
    UploadMessageReturnState tempUploadMessageReturnState = UploadMessageReturnState::OK ;

    //  检查uploadedMessage
    tempUploadMessageReturnState == UploadMessageReturnState::OK;
    if (tempUploadMessageReturnState != UploadMessageReturnState::OK)
    {
        qWarning()<<"DBusModule::UploadMessage: uploadMessage="<<uploadedMessage<<" is invalid";
        return tempUploadMessageReturnState;
    }
    qDebug()<<"checkUploadedMessage";
    //  检查uploadedMessageSha256
//    tempUploadMessageReturnState = checkHashedValue(uploadedMessage,uploadedMessageSha256);

    if(tempUploadMessageReturnState != UploadMessageReturnState::OK)
    {
        qWarning()<<"DBusModule::UploadMessage: uploadMessageSha256="<<uploadedMessageSha256<<" is invalid";
        return tempUploadMessageReturnState;
    }
    qDebug()<<"checkHasedValue" << m_uploadState;

    if(m_uploadState == KylinDaqCommon::UploadState::UploadEnable)
    {
        qInfo()<<"DBusModule::UploadEventMessage: saveEventInfo";
        emit saveEventInfo(uploadedMessage);
    }
    return tempUploadMessageReturnState;
}


int DBusModule::UploadMessage(const QString &packageInfo, const QString &uploadedMessage, const QString &uploadedMessageSha256, QString &retTid)
{
    QString source;
    QString tid;
    UploadMessageReturnState tempUploadMessageReturnState = UploadMessageReturnState::OK;
    retTid = "";
    // 检测输入参数
    tempUploadMessageReturnState = UploadMessageReturnState::OK;
    // tempUploadMessageReturnState = checkUploadMessage(packageInfo,uploadedMessage,uploadedMessageSha256);
    if (tempUploadMessageReturnState != UploadMessageReturnState::OK)
    {
        if(tempUploadMessageReturnState == UploadMessageReturnState::InvalidTid)
        {
            retTid = createPackInfoId(packageInfo);
            qDebug()<<__FUNCTION__<<"now tid:"<<retTid;
        }
        else {
            qDebug()<<__FUNCTION__<<"return invalid code:"<<tempUploadMessageReturnState;
            return tempUploadMessageReturnState;
        }
    }
    //  存储消息
    //	填充source
    tid = getPackageInfoTid(packageInfo);
    source = fillSource(packageInfo,tid,retTid);
    QString validInfo = fillValidInfo(source,uploadedMessage);
    // qDebug() << "sendInfo="<<validInfo;
    if(m_uploadState == KylinDaqCommon::UploadState::UploadEnable)
    {
        if (m_filterState == SetState::Valid)
        {
            QString packageName;
            QString messageType;
            getPackageInfo(packageInfo,packageName,messageType);
            foreach(QString validPackageName,m_validPackageName)
            {
                if(packageName == validPackageName)
                {
                    emit saveInfo(validInfo);
                    qInfo()<<"DBusModule::UploadMessage: save info";
                    break;
                }
            }
        }
        else
        {
            emit saveInfo(validInfo);
            qInfo()<<"DBusModule::UploadMessage: save info";
        }
    }
    if(tempUploadMessageReturnState == UploadMessageReturnState::InvalidTid)
    {
        qInfo()<<"DBusModule::UploadMessage: save tid";
        emit saveTid(retTid);
    }
    //	返回状态
    return tempUploadMessageReturnState;
}


int DBusModule::UploadStateMessage(const QString &packageInfo, const QString &uploadedMessage, const QString &uploadedMessageSha256, const int32_t &uploadType, QString &retTid)
{
    QString source;
    QString tid;
    UploadMessageReturnState tempUploadMessageReturnState = UploadMessageReturnState::OK ;
    // 重置输出packInfoId
    retTid = "";
    tempUploadMessageReturnState = UploadMessageReturnState::OK;
    // tempUploadMessageReturnState = checkUploadStateMessage(packageInfo,uploadedMessage,uploadedMessageSha256, KylinDaqCommon::UploadStateEnum(uploadType));
    qDebug()<<__FUNCTION__<<"packageInfo";
    if(tempUploadMessageReturnState != UploadMessageReturnState::OK)
    {
        if(tempUploadMessageReturnState == UploadMessageReturnState::InvalidTid)
        {
            retTid = createPackInfoId(packageInfo);
            qDebug()<<__FUNCTION__<<retTid;
        }
        else {
            return tempUploadMessageReturnState;
        }
    }
    qDebug()<<__FUNCTION__<<"tid";
    //  存储消息
    //	填充source
    tid = getPackageInfoTid(packageInfo);
    source = fillSource(packageInfo,tid,retTid);
    KylinDaqCommon::UploadStateMessageInfo_t uploadStateMessageInfo;
    KylinDaqCommon::UploadStateMessageInfo_t oldUploadStateMessageInfo;
    createStateMessageInfo(packageInfo, uploadedMessage, uploadStateMessageInfo);
    oldUploadStateMessageInfo.packageInfo = uploadStateMessageInfo.packageInfo;

    getHisotryStateMessage(oldUploadStateMessageInfo.packageInfo,
    oldUploadStateMessageInfo.uploadedMessage,
    oldUploadStateMessageInfo.tids,
    oldUploadStateMessageInfo.createTimeStamp,
    oldUploadStateMessageInfo.uploadState);
    if (oldUploadStateMessageInfo.uploadedMessage == uploadStateMessageInfo.uploadedMessage) {
        return UploadMessageReturnState::OK;
    }
    // qDebug() << "sendInfo="<<validInfo;
    if(m_uploadState == KylinDaqCommon::UploadState::UploadEnable)
    {
        if (m_filterState == SetState::Valid)
        {
            QString packageName;
            QString messageType;
            getPackageInfo(packageInfo,packageName,messageType);
            foreach(QString validPackageName,m_validPackageName)
            {
                if(packageName == validPackageName)
                {
                    saveStateMessageInfo(uploadStateMessageInfo,oldUploadStateMessageInfo,KylinDaqCommon::UploadTypeEnum(uploadType));
                    qInfo()<<"DBusModule::UploadStateMessage: save info";
                    break;
                }
            }
        }
        else
        {
            saveStateMessageInfo(uploadStateMessageInfo,oldUploadStateMessageInfo,KylinDaqCommon::UploadTypeEnum(uploadType));
            qInfo()<<"DBusModule::UploadStateMessage: save info";
        }
    }
    if(tempUploadMessageReturnState == UploadMessageReturnState::InvalidTid)
    {
        qInfo()<<"DBusModule::UploadStateMessage: save tid";
        emit saveTid(retTid);
    }
    //	返回状态
    return tempUploadMessageReturnState;
}

QString DBusModule::GetTid(const QString &packageName, const QString &messageType)
{
    bool existState = false;
    QString validString = packageName + messageType;
    QCryptographicHash cryHash(QCryptographicHash::Algorithm::Sha256);
    cryHash.addData(validString.toUtf8());
    QString nowPackInfoId = cryHash.result().toBase64();
    emit checkTid(nowPackInfoId,existState);
    if(existState == true)
    {
        return nowPackInfoId;
    }
    else
    {
        return "";
    }
}

void DBusModule::updateProperty(const QJsonObject &conf, const QJsonObject &defaultConf)
{
    qlonglong value;
    QList<QString> tempStrList;
    KylinDaqCommon::setValidConfInt(conf, defaultConf, s_transmitModuleName, s_transmitModuleUploadStateKey, value);
    setUploadState(KylinDaqCommon::UploadState(value));
    KylinDaqCommon::setValidConfInt(conf, defaultConf, moduleName(), s_filterStateKey, value);
    setFilterState(SetState(value));
    KylinDaqCommon::setValidConfStrArray(conf, defaultConf, moduleName(), s_validPackageNameKey, tempStrList);
    setValidPackageName(tempStrList);
}


DBusModule::UploadMessageReturnState DBusModule::checkUploadMessage(const QString &packageInfo, const QString &uploadedMessage, const QString &uploadedMessageSha256)
{
    UploadMessageReturnState tempUploadMessageReturnState = UploadMessageReturnState::OK ;
    //  检查uploadedMessage
    qDebug()<<__FUNCTION__<<"checkUploadedMessage";
    tempUploadMessageReturnState = checkUploadedMessage(uploadedMessage);
    if (tempUploadMessageReturnState != UploadMessageReturnState::OK)
    {
        qWarning()<<"DBusModule::UploadMessage: uploadMessage="<<uploadedMessage<<" is invalid";
        return tempUploadMessageReturnState;
    }
    //  检查uploadedMessageSha256
    // qDebug()<<__FUNCTION__<<"checkcheckHashedValue";
    // tempUploadMessageReturnState = checkHashedValue(uploadedMessage,uploadedMessageSha256);
    // if(tempUploadMessageReturnState != UploadMessageReturnState::OK)
    // {
    //     qWarning()<<"DBusModule::UploadMessage: uploadMessageSha256="<<uploadedMessageSha256<<" is invalid";
    //     return tempUploadMessageReturnState;
    // }
    //  检查packInfo
    qDebug()<<__FUNCTION__<<"packageInfo";
    tempUploadMessageReturnState = checkPackageInfo(packageInfo);
    return tempUploadMessageReturnState;
}

DBusModule::UploadMessageReturnState DBusModule::checkUploadStateMessage(const QString &packageInfo, const QString &uploadedMessage, const QString &uploadedMessageSha256, const KylinDaqCommon::UploadStateEnum &uploadType)
{
    UploadMessageReturnState tempUploadMessageReturnState1 = UploadMessageReturnState::OK;
    UploadMessageReturnState tempUploadMessageReturnState2 = UploadMessageReturnState::OK;
    tempUploadMessageReturnState1 = checkUploadMessage(packageInfo,uploadedMessage,uploadedMessageSha256);
    if (tempUploadMessageReturnState1 != UploadMessageReturnState::OK || tempUploadMessageReturnState1 != UploadMessageReturnState::InvalidTid) {
        return tempUploadMessageReturnState1;
    }
    tempUploadMessageReturnState2 = checkUploadType(uploadType);
    if (tempUploadMessageReturnState2 != UploadMessageReturnState::OK ) {
        return tempUploadMessageReturnState2;
    }
    return tempUploadMessageReturnState1;

}

DBusModule::UploadMessageReturnState DBusModule::checkUploadType(const int32_t &uploadType)
{
    UploadMessageReturnState tempRetState = UploadMessageReturnState::OK;
    if (uploadType < 0 || uploadType > 2) {
       tempRetState = UploadMessageReturnState::UploadTypeInvalid;
    }
    return tempRetState;
}

DBusModule::UploadMessageReturnState DBusModule::createStateMessageInfo(const QString &packageInfo, const QString &uploadedMessage, KylinDaqCommon::UploadStateMessageInfo_t &uploadStateMessageInfo)
{
    UploadMessageReturnState retState = UploadMessageReturnState::OK;
    QJsonObject tempPackageInfoJsonObj = QJsonDocument::fromJson(packageInfo.toUtf8()).object();
    QJsonObject uploadMessageJsonObj = QJsonDocument::fromJson(uploadedMessage.toUtf8()).object();
    QJsonObject tidsJsonObj;
    QJsonObject createTimeStampJsonObj;
    QJsonObject packageInfoJsonObj;

    if (tempPackageInfoJsonObj.contains(KylinDaqCommon::gs_packageNameKey))
    {
        packageInfoJsonObj[KylinDaqCommon::gs_packageNameKey] = tempPackageInfoJsonObj[KylinDaqCommon::gs_packageNameKey];
    }
    if (tempPackageInfoJsonObj.contains(KylinDaqCommon::gs_messageTypeKey))
    {
        packageInfoJsonObj[KylinDaqCommon::gs_messageTypeKey] = tempPackageInfoJsonObj[KylinDaqCommon::gs_messageTypeKey];
    }
    if (tempPackageInfoJsonObj.contains(KylinDaqCommon::gs_tidKey))
    {
        tidsJsonObj[KylinDaqCommon::gs_tidKey] = tempPackageInfoJsonObj[KylinDaqCommon::gs_tidKey];
    }
    if (tempPackageInfoJsonObj.contains(KylinDaqCommon::gs_parentTidKey))
    {
        tidsJsonObj[KylinDaqCommon::gs_parentTidKey] = tempPackageInfoJsonObj[KylinDaqCommon::gs_parentTidKey];
    }
    if (uploadMessageJsonObj.contains(KylinDaqCommon::gs_createTimeStampKey)) {
        createTimeStampJsonObj[KylinDaqCommon::gs_createTimeStampKey] = uploadMessageJsonObj[KylinDaqCommon::gs_createTimeStampKey];
        uploadMessageJsonObj.remove(KylinDaqCommon::gs_createTimeStampKey);
    }
    uploadStateMessageInfo.packageInfo = QString(QJsonDocument(packageInfoJsonObj).toJson(QJsonDocument::Compact));
    uploadStateMessageInfo.uploadedMessage = QString(QJsonDocument(uploadMessageJsonObj).toJson(QJsonDocument::Compact));
    uploadStateMessageInfo.tids = QString(QJsonDocument(tidsJsonObj).toJson(QJsonDocument::Compact));
    uploadStateMessageInfo.createTimeStamp = QString(QJsonDocument(createTimeStampJsonObj).toJson(QJsonDocument::Compact));
    return retState;
}

QString DBusModule::createMessage(KylinDaqCommon::UploadStateMessageInfo_t &uploadStateMessageInfo)
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

void DBusModule::saveStateMessageInfo(KylinDaqCommon::UploadStateMessageInfo_t &uploadStateMessageInfo,KylinDaqCommon::UploadStateMessageInfo_t &oldUploadStateMessageInfo,const KylinDaqCommon::UploadTypeEnum &uploadType)
{
    qInfo()<<"DBusModule::UploadStateMessage: uploadType="<<uploadType;
    if (uploadType == KylinDaqCommon::UploadTypeEnum::StateAndTwoMessage && !oldUploadStateMessageInfo.uploadedMessage.isEmpty())
    {
        saveInfo(createMessage(oldUploadStateMessageInfo));
        qInfo()<<"DBusModule::UploadStateMessage: save event info";
    }
    if (uploadType == KylinDaqCommon::UploadTypeEnum::StateAndTwoMessage || uploadType == KylinDaqCommon::UploadTypeEnum::StateAndMessage)
    {
        saveInfo(createMessage(uploadStateMessageInfo));
        qInfo()<<"DBusModule::UploadStateMessage: save event info";
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
        qInfo()<<"DBusModule::UploadStateMessage: save state info";
    }
    else
    {
        updateStateMessage(uploadStateMessageInfo.packageInfo,
        uploadStateMessageInfo.uploadedMessage,
        uploadStateMessageInfo.tids,
        uploadStateMessageInfo.createTimeStamp,
        uploadStateMessageInfo.uploadState);
        qInfo()<<"DBusModule::UploadStateMessage: update state info";
    }

}