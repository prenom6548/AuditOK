/*
 * Copyright (C) 2023 KylinSoftCo., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 * 
**/
#include "bioauth.h"
#include <QList>
#include <unistd.h>

BioAuth::BioAuth(qint32 uid, const DeviceInfoPtr deviceInfo, QObject *parent)
    : QObject(parent),
      uid(uid),
      deviceInfo(deviceInfo),
      isInAuthentication(false)
{
    init();
}

BioAuth::BioAuth(QObject *parent)
    : QObject(parent)
    , isInAuthentication(false)
{
    init();
}

void BioAuth::init()
{
    serviceInterface = new QDBusInterface(BIO_DBUS_SERVICE,
                                          BIO_DBUS_PATH,
                                          BIO_DBUS_INTERFACE,
                                          QDBusConnection::systemBus());

    connect(serviceInterface, SIGNAL(StatusChanged(int, int)),
            this, SLOT(onStatusChanged(int,int)));

    connect(serviceInterface, SIGNAL(FrameWritten(int)),
            this, SLOT(onFrameWritten(int)));

    serviceInterface->setTimeout(2147483647);
}

BioAuth::~BioAuth()
{
  //  stopAuth();
}

void BioAuth::setDevice(const DeviceInfoPtr deviceInfo)
{
    this->deviceInfo = deviceInfo;
}

void BioAuth::setUid(qint32 uid)
{
    this->uid = uid;
}

void BioAuth::startUkeyAuth()
{
    //    stopAuth();
    if (!deviceInfo) {
        qDebug()<<"DeviceInfo not valid!!";
        return;
    }

    /* 开始认证识别 */
    LOG() << "start biometric verification";
    QList<QVariant> args;
    args << QVariant(deviceInfo->device_id) << QVariant(2)
         << QVariant(uid);
    isInAuthentication = true;

    QDBusPendingCall call = serviceInterface->asyncCallWithArgumentList("UkeyIdentify", args);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, &BioAuth::onIdentityComplete);
}
DeviceInfoPtr BioAuth::getDevice()
{
    return this->deviceInfo;
}

void BioAuth::startAuth()
{
//    stopAuth();
    if (!deviceInfo) {
        qDebug()<<"DeviceInfo not valid!!";
        return;
    }

    /* 开始认证识别 */
    LOG() << "start biometric verification";
    QList<QVariant> args;
    args << QVariant(deviceInfo->device_id) << QVariant(uid)
         << QVariant(0) << QVariant(-1);
    isInAuthentication = true;

    qDebug()<<args;

    if(deviceInfo->biotype == LOGINOPT_TYPE_GENERAL_UKEY){
        QList<QVariant> args1;
        args1 << QVariant(deviceInfo->device_id) << QVariant(2)
             << QVariant(uid);

        QDBusPendingCall call = serviceInterface->asyncCallWithArgumentList("UkeyIdentify", args1);
        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this, &BioAuth::onIdentityComplete);
    }else{
        //使用sudo等命令时，不停止设备，当存在其他可认证设备时，使用其他可认证设备，否则使用密码
        if(qAppName() != "bioauth"){
            //stopops在identify之前使用时，不要调用getFeaturelist，否则会出现identify被停止的现象，应该时框架的bug。
            serviceInterface->call("StopOps", QVariant(deviceInfo->device_id), QVariant(3000));
        }
        QDBusPendingCall call = serviceInterface->asyncCallWithArgumentList("Identify", args);
        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this, &BioAuth::onIdentityComplete);
    }
}

bool BioAuth::GetHasUkeyFeature(int uid, int indexStart, int indexEnd)
{
    QList<QDBusVariant> qlist;
    FeatureInfo *featureInfo;
    int listsize;
    QDBusMessage result = serviceInterface->call(QStringLiteral("GetAllFeatureList"),uid,indexStart,indexEnd);
    if(result.type() == QDBusMessage::ErrorMessage)
    {
        qWarning() << "GetDevList error:" << result.errorMessage();
        return false;
    }
    QList<QVariant> variantList = result.arguments();
    listsize = variantList[0].value<int>();
    variantList[1].value<QDBusArgument>() >> qlist;
    for (int i = 0; i < listsize; i++) {
        featureInfo = new FeatureInfo;
        qlist[i].variant().value<QDBusArgument>() >> *featureInfo;
        if(featureInfo->biotype == LOGINOPT_TYPE_GENERAL_UKEY){
            delete featureInfo;
            return true;
        }
        delete featureInfo;
    }
    return false;
}

int BioAuth::SetExtraInfo(QString info_type,QString extra_info)
{
    QDBusReply<int> reply = serviceInterface->call(QStringLiteral("SetExtraInfo"), info_type, extra_info);
    if(!reply.isValid())
    {
        qWarning() << "SetExtraInfo error:" << reply.error();
        return -1;
    }
    return reply.value();
}

void BioAuth::startAuth(qint32 uid, const DeviceInfoPtr deviceInfo)
{
    if (!deviceInfo) {
        return ;
    }
    this->uid = uid;
    this->deviceInfo = deviceInfo;
    startAuth();
}

void BioAuth::stopAuth()
{
    if(!isInAuthentication)
        return ;

    if (!deviceInfo) {
        qDebug()<<"DeviceInfo not valid!!";
        return;
    }

    QDBusReply<int> reply = serviceInterface->call("StopOps", QVariant(deviceInfo->device_id), QVariant(3000));

    emit authFinished();
    if(!reply.isValid())
        qWarning() << "StopOps error: " << reply.error();

    isInAuthentication = false;
}

bool BioAuth::isAuthenticating()
{
    return isInAuthentication;
}

void BioAuth::onIdentityComplete(QDBusPendingCallWatcher *watcher)
{
    emit authFinished();
    QDBusPendingReply<qint32, qint32> reply = *watcher;
    if(reply.isError()){
        isInAuthentication = false;
        LOG() << reply.error();
        Q_EMIT authComplete(-1, false, -1);
        return;
    }
    qint32 result = reply.argumentAt(0).toInt();
    qint32 retUid = reply.argumentAt(1).toInt();
    LOG() << "Identify complete: " << result << " " << retUid << " " << uid;

    /* 识别生物特征成功，发送认证结果 */
    if(isInAuthentication){
        isInAuthentication = false;

        if(result == DBUS_RESULT_SUCCESS && retUid == uid){
            Q_EMIT authComplete(retUid, true, result);
        }else {
            Q_EMIT authComplete(uid, false, result);
        }
    }
}

void BioAuth::onFrameWritten(int deviceId)
{
    Q_EMIT frameWritten(deviceId);
}

void BioAuth::onStatusChanged(int deviceId, int statusType)
{
    if(statusType != STATUS_NOTIFY)
        return;
    LOG() << "status changed " << deviceId << " " << statusType;
    QDBusMessage msg = serviceInterface->call("GetNotifyMesg", QVariant(deviceId));
    if(msg.type() == QDBusMessage::ErrorMessage){
        LOG() << msg.errorMessage();
        return;
    }

    if(deviceInfo && deviceInfo->device_id != deviceId){
        return ;
    }

    QString message = msg.arguments().at(0).toString();
    LOG() << message;
    Q_EMIT notify(message);
    Q_EMIT notifyDetail(deviceId, message);
}

