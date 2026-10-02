/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
#include "biometrichelper.h"
#include <QDebug>
#include <QDBusArgument>

BiometricHelper::BiometricHelper(QObject *parent)
    : QDBusAbstractInterface(
        BIOMETRIC_DBUS_SERVICE, BIOMETRIC_DBUS_PATH, BIOMETRIC_DBUS_INTERFACE, QDBusConnection::systemBus(), parent)
{
    setTimeout(2147483647);
}

QDBusPendingCall BiometricHelper::Identify(int drvid, int uid, int indexStart, int indexEnd)
{
    QList<QVariant> argList;
    argList << drvid << uid << indexStart << indexEnd;
    return asyncCallWithArgumentList(QStringLiteral("Identify"), argList);
}

QDBusPendingCall BiometricHelper::UkeyIdentify(int drvid, int type, int uid)
{
    QList<QVariant> argList;
    argList << drvid << type << uid;
    return asyncCallWithArgumentList(QStringLiteral("UkeyIdentify"), argList);
}

bool BiometricHelper::GetHasUkeyFeature(int uid, int indexStart, int indexEnd)
{
    QList<QDBusVariant> qlist;
    FeatureInfo *featureInfo;
    int listsize;
    QDBusMessage result = call(QStringLiteral("GetAllFeatureList"), uid, indexStart, indexEnd);
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "GetDevList error:" << result.errorMessage();
        return false;
    }
    QList<QVariant> variantList = result.arguments();
    listsize = variantList[0].value<int>();
    variantList[1].value<QDBusArgument>() >> qlist;
    for (int i = 0; i < listsize; i++) {
        featureInfo = new FeatureInfo;
        qlist[i].variant().value<QDBusArgument>() >> *featureInfo;
        if (featureInfo->biotype == UniT_General_Ukey) {
            delete featureInfo;
            return true;
        }
        delete featureInfo;
    }

    return false;
}

int BiometricHelper::GetFeatureCount(int uid, int indexStart, int indexEnd)
{
    QDBusMessage result = call(QStringLiteral("GetDevList"));
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "GetDevList error:" << result.errorMessage();
        return 0;
    }
    auto dbusArg = result.arguments().at(1).value<QDBusArgument>();
    QList<QVariant> variantList;
    dbusArg >> variantList;
    int res = 0;
    for (int i = 0; i < variantList.size(); i++) {

        DeviceInfoPtr pDeviceInfo = std::make_shared<DeviceInfo>();

        auto arg = variantList.at(i).value<QDBusArgument>();
        arg >> *pDeviceInfo;

        QDBusMessage FeatureResult = call(QStringLiteral("GetFeatureList"), pDeviceInfo->id, uid, indexStart, indexEnd);
        if (FeatureResult.type() == QDBusMessage::ErrorMessage) {
            qWarning() << "GetFeatureList error:" << FeatureResult.errorMessage();
            return 0;
        }
        res += FeatureResult.arguments().takeFirst().toInt();
    }
    return res;
}

int BiometricHelper::SetExtraInfo(QString info_type, QString extra_info)
{
    QDBusReply<int> reply = call(QStringLiteral("SetExtraInfo"), info_type, extra_info);
    if (!reply.isValid()) {
        qWarning() << "SetExtraInfo error:" << reply.error();
        return -1;
    }
    return reply.value();
}

int BiometricHelper::StopOps(int drvid, int waiting)
{
    QDBusReply<int> reply = call(QStringLiteral("StopOps"), drvid, waiting);
    if (!reply.isValid()) {
        qWarning() << "StopOps error:" << reply.error();
        return -1;
    }
    return reply.value();
}

int BiometricHelper::GetUserDevCount(int uid)
{
    QDBusMessage result = call(QStringLiteral("GetDevList"));
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "GetDevList error:" << result.errorMessage();
        return 0;
    }
    auto dbusArg = result.arguments().at(1).value<QDBusArgument>();
    QList<QVariant> variantList;
    DeviceList deviceList;
    dbusArg >> variantList;
    for (int i = 0; i < variantList.size(); i++) {
        DeviceInfoPtr pDeviceInfo = std::make_shared<DeviceInfo>();

        auto arg = variantList.at(i).value<QDBusArgument>();
        arg >> *pDeviceInfo;

        int count = GetUserDevFeatureCount(uid, pDeviceInfo->id);

        if (count > 0)
            deviceList.push_back(pDeviceInfo);
    }

    return deviceList.count();
}

int BiometricHelper::GetUserDevFeatureCount(int uid, int drvid)
{
    StopOps(drvid);
    QDBusMessage FeatureResult = call(QStringLiteral("GetFeatureList"), drvid, uid, 0, -1);
    if (FeatureResult.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "GetFeatureList error:" << FeatureResult.errorMessage();
        return 0;
    }
    return FeatureResult.arguments().takeFirst().toInt();
}

DeviceList BiometricHelper::GetDevList()
{
    QDBusMessage result = call(QStringLiteral("GetDevList"));
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "GetDevList error:" << result.errorMessage();
        return DeviceList();
    }
    auto dbusArg = result.arguments().at(1).value<QDBusArgument>();
    QList<QVariant> variantList;
    DeviceList deviceList;
    dbusArg >> variantList;
    for (int i = 0; i < variantList.size(); i++) {
        DeviceInfoPtr pDeviceInfo = std::make_shared<DeviceInfo>();

        auto arg = variantList.at(i).value<QDBusArgument>();
        arg >> *pDeviceInfo;

        deviceList.push_back(pDeviceInfo);
    }

    return deviceList;
}

FeatureMap BiometricHelper::GetUserFeatures(int uid)
{
    FeatureMap featureMap;
    QList<QDBusVariant> qlist;
    int listsize;
    QDBusMessage result = call(QStringLiteral("GetAllFeatureList"), uid, 0, -1);
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "GetDevList error:" << result.errorMessage();
        return featureMap;
    }
    QList<QVariant> variantList = result.arguments();
    listsize = variantList[0].value<int>();
    variantList[1].value<QDBusArgument>() >> qlist;
    for (int i = 0; i < listsize; i++) {
        FeatureInfoPtr pFeatureInfo = std::make_shared<FeatureInfo>();
        qlist[i].variant().value<QDBusArgument>() >> *pFeatureInfo;
        featureMap[pFeatureInfo->device_shortname].append(pFeatureInfo);
    }
    return featureMap;
}

int BiometricHelper::GetDevCount()
{
    QDBusMessage result = call(QStringLiteral("GetDevList"));
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "GetDevList error:" << result.errorMessage();
        return 0;
    }
    int count = result.arguments().at(0).value<int>();
    return count;
}

QString BiometricHelper::GetDevMesg(int drvid)
{
    QDBusMessage result = call(QStringLiteral("GetDevMesg"), drvid);
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "GetDevMesg error:" << result.errorMessage();
        return "";
    }
    return result.arguments().at(0).toString();
}

QString BiometricHelper::GetNotifyMesg(int drvid)
{
    QDBusMessage result = call(QStringLiteral("GetNotifyMesg"), drvid);
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "GetNotifyMesg error:" << result.errorMessage();
        return "";
    }
    return result.arguments().at(0).toString();
}

QString BiometricHelper::GetOpsMesg(int drvid)
{
    QDBusMessage result = call(QStringLiteral("GetOpsMesg"), drvid);
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "GetOpsMesg error:" << result.errorMessage();
        return "";
    }
    return result.arguments().at(0).toString();
}

StatusReslut BiometricHelper::UpdateStatus(int drvid)
{
    StatusReslut status;
    QDBusMessage result = call(QStringLiteral("UpdateStatus"), drvid);
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "UpdateStatus error:" << result.errorMessage();
        status.result = -1;
        return status;
    }

    status.result = result.arguments().at(0).toInt();
    status.enable = result.arguments().at(1).toInt();
    status.devNum = result.arguments().at(2).toInt();
    status.devStatus = result.arguments().at(3).toInt();
    status.opsStatus = result.arguments().at(4).toInt();
    status.notifyMessageId = result.arguments().at(5).toInt();

    return status;
}
