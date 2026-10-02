/*
 * Copyright (C) 2013 ~ 2021 KylinSoft Ltd.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "dbus_client.h"
#include "displayinfo.h"

#include <sys/syslog.h>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonParseError>
#include <QDebug>



namespace kmre {

DBusClient* DBusClient::mDBusClient = nullptr;

DBusClient* DBusClient::getInstance()
{
    if (!mDBusClient) {
        mDBusClient = new DBusClient();
    }
    return mDBusClient;
}

DBusClient::DBusClient()
{
    mSystemBusInterface = new QDBusInterface("cn.kylinos.Kmre", "/cn/kylinos/Kmre", "cn.kylinos.Kmre", QDBusConnection::systemBus());
    mSessionBusInterface = new QDBusInterface("cn.kylinos.Kmre.Manager", "/cn/kylinos/Kmre/Manager", "cn.kylinos.Kmre.Manager", QDBusConnection::sessionBus());
    mKmreWindowInterface = new QDBusInterface("cn.kylinos.Kmre.Window", "/cn/kylinos/Kmre/Window", "cn.kylinos.Kmre.Window", QDBusConnection::sessionBus());
}

DBusClient::~DBusClient()
{
    if (mSystemBusInterface) {
        delete mSystemBusInterface;
        mSystemBusInterface = nullptr;
    }

    if (mSessionBusInterface) {
        delete mSessionBusInterface;
        mSessionBusInterface = nullptr;
    }
}

int32_t DBusClient::Prepare(const QString& userName, int32_t uid)
{
    int ret = -1;
    QDBusMessage response = mSystemBusInterface->call("Prepare", userName, uid);
    if (response.type() == QDBusMessage::ReplyMessage) {
        ret = response.arguments().takeFirst().toInt();
    }

    return ret;
}

int32_t DBusClient::StartContainer(const QString& userName, int32_t uid, int32_t width, int32_t height)
{
    int ret = -1;
    QDBusMessage response = mSystemBusInterface->call("StartContainer", userName, uid, width, height);
    if (response.type() == QDBusMessage::ReplyMessage) {
        ret = response.arguments().takeFirst().toInt();
    }
    return ret;
}

int32_t DBusClient::StartContainerSilently(const QString& userName, int32_t uid)
{
    int ret = -1;
    QDBusMessage response = mSystemBusInterface->call("StartContainerSilently", userName, uid);
    if (response.type() == QDBusMessage::ReplyMessage) {
        ret = response.arguments().takeFirst().toInt();
    }

    return ret;
}

int32_t DBusClient::ChangeContainerRuntimeStatus(const QString& userName, int32_t uid)
{
    int ret = -1;
    QDBusMessage response = mSystemBusInterface->call("ChangeContainerRuntimeStatus", userName, uid);
    if (response.type() == QDBusMessage::ReplyMessage) {
        ret = response.arguments().takeFirst().toInt();
    }

    return ret;
}

// void DBusClient::SetFocusOnContainer(const QString& userName, int32_t uid, int32_t onFocus)
// {
//     mSystemBusInterface->call("SetFocusOnContainer", userName, uid, onFocus);
// }

QString DBusClient::GetPropOfContainer(const QString& userName, int32_t uid, const QString& prop)
{
    QString value;
    QDBusMessage response = mSystemBusInterface->call("GetPropOfContainer", userName, uid, prop);
    if (response.type() == QDBusMessage::ReplyMessage) {
        value = response.arguments().takeFirst().toString();
    }
    return value;
}

void DBusClient::SetPropOfContainer(const QString &userName, int32_t uid, const QString &prop, const QString &value)
{
    mSystemBusInterface->call("SetPropOfContainer", userName, uid, prop, value);
}

int32_t DBusClient::IsImageReady()
{
    unsigned int ready = 1;
    QDBusMessage response = mSystemBusInterface->call("IsImageReady");
    if (response.type() == QDBusMessage::ReplyMessage) {
        ready = response.arguments().takeFirst().toInt();
    }
    return ready;
}

void DBusClient::LoadImage()
{
    mSystemBusInterface->call("LoadImage");
}

bool DBusClient::GetDisplayInformation()
{
    QString displayInfo;
    QDBusMessage response = mSessionBusInterface->call("getDisplayInformation");
    if (response.type() == QDBusMessage::ReplyMessage) {
        displayInfo = response.arguments().takeFirst().toString();
    }
    else {
        syslog(LOG_ERR, "Dbus: getDisplayInformation method called failed!");
        qDebug() << "Dbus: getDisplayInformation method called failed!";
        return false;
    }
    
    if (!displayInfo.isEmpty()) {
        QJsonDocument jsonDocument = QJsonDocument::fromJson(displayInfo.toLocal8Bit().data());
        if (!jsonDocument.isNull()) {
            QJsonObject jsonObject = jsonDocument.object();
            if (!jsonObject.isEmpty() && jsonObject.size() > 0) {
                if (jsonObject.contains("display_type")) {
                    gDisplayInfo.displayType = jsonObject.value("display_type").toString();
                }
                if (jsonObject.contains("cpu_type")) {
                    gDisplayInfo.cpuType = jsonObject.value("cpu_type").toString();
                }
                if (jsonObject.contains("cpu_supported")) {
                    gDisplayInfo.cpuSupported = jsonObject.value("cpu_supported").toBool();
                }
                if (jsonObject.contains("gpu_vendor")) {
                    gDisplayInfo.gpuVendor = jsonObject.value("gpu_vendor").toString();
                }
                if (jsonObject.contains("gpu_model")) {
                    gDisplayInfo.gpuModel = jsonObject.value("gpu_model").toString();
                }
                if (jsonObject.contains("gpu_supported")) {
                    gDisplayInfo.gpuSupported = jsonObject.value("gpu_supported").toBool();
                }
                if (jsonObject.contains("physical_display.height")) {
                    gDisplayInfo.physicalHeight = jsonObject.value("physical_display.height").toInt();
                }
                if (jsonObject.contains("physical_display.width")) {
                    gDisplayInfo.physicalWidth = jsonObject.value("physical_display.width").toInt();
                }
                if (jsonObject.contains("window_display.height")) {
                    gDisplayInfo.displayHeight = jsonObject.value("window_display.height").toInt();
                }
                if (jsonObject.contains("window_display.width")) {
                    gDisplayInfo.displayWidth = jsonObject.value("window_display.width").toInt();
                }
                if (jsonObject.contains("physical_display.density")) {
                    gDisplayInfo.density = jsonObject.value("physical_display.density").toInt();
                }
            }
        }
    }

    return true;
}

bool DBusClient::isHostSupportDDS()
{
    bool value = false;
    QDBusMessage response = mSessionBusInterface->call("isHostSupportDDS");
    if (response.type() == QDBusMessage::ReplyMessage) {
        value = response.arguments().takeFirst().toBool();
    }
    return value;
}

void DBusClient::StartKmreManager()
{
    mSessionBusInterface->call("start");
}

void DBusClient::StartKmreWindow()
{
    mKmreWindowInterface->call("start");
}

void DBusClient::StartUserService(const QString& name, const QString& path, const QString& interface)
{
    QDBusInterface i(name, path, interface, QDBusConnection::sessionBus());
    i.call("start");
}

QStringList DBusClient::GetRunningAppList()
{
    QString valueStr;
    QDBusMessage response = mKmreWindowInterface->call("getRunningAppList");
    if (response.type() == QDBusMessage::ReplyMessage) {
        valueStr = response.arguments().takeFirst().toString();
    }

    return valueStr.split(",", QString::SkipEmptyParts);
}

void DBusClient::LaunchApp(const QString& pkgName, int width, int height)
{
    mKmreWindowInterface->call("launchApp", pkgName, width, height);
}

void DBusClient::ActiveApp(const QString& pkgName)
{
    mKmreWindowInterface->call("activeApp", pkgName);
}

} // namespace kmre
