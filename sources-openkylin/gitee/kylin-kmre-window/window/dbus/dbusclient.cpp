/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  Alan Xie    xiehuijun@kylinos.cn
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

#include "dbusclient.h"
#include "kmreenv.h"
#include "screencapturedbus.h"

#include <sys/syslog.h>
#include <unistd.h>
#include <QMutexLocker>
#include <QDebug>
#include <QStandardPaths>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusConnectionInterface>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusReply>
#include <QTimer>
namespace kmre {

DbusClient::DbusClient(QObject *parent)
    : QObject(parent)
{
    mKmreDBusInterface = std::make_unique<QDBusInterface>("cn.kylinos.Kmre",
                                                            "/cn/kylinos/Kmre",
                                                            "cn.kylinos.Kmre",
                                                            QDBusConnection::systemBus());
    if (!mKmreDBusInterface->isValid()) {
        syslog(LOG_ERR, "[%s] Kmre dbus interface is invalid!", __func__);
    }

    mManagerDBusInterface = std::make_unique<QDBusInterface>("cn.kylinos.Kmre.Manager",
                                                            "/cn/kylinos/Kmre/Manager",
                                                            "cn.kylinos.Kmre.Manager",
                                                            QDBusConnection::sessionBus());
    if (!mManagerDBusInterface->isValid()) {
        syslog(LOG_ERR, "[%s] Manager dbus interface is invalid!", __func__);
    }
}

DbusClient::~DbusClient()
{
}

int32_t DbusClient::Prepare(const QString &userName, int32_t uid)
{
    if (mKmreDBusInterface->isValid()) {
        QDBusMessage response = mKmreDBusInterface->call("Prepare", userName, uid);
        if (response.type() == QDBusMessage::ReplyMessage) {
            return response.arguments().takeFirst().toInt();
        }
    }

    return -1;
}

void DbusClient::StopContainer(const QString &userName, int uid)
{
    if (mKmreDBusInterface->isValid()) {
        // ignore reply, will recive signal: 'Stopped' when conatiner stopped.
        mKmreDBusInterface->asyncCall("StopContainer", userName, uid);
    }
}

// 给容器中的android环境设置指定属性的值
void DbusClient::SetPropOfContainer(const QString &userName, int32_t uid, const QString &prop, const QString &value)
{
    if (mKmreDBusInterface->isValid()) {
        mKmreDBusInterface->call("SetPropOfContainer", userName, uid, prop, value);
    }
}

// 从容器中的android环境获取默认属性
QString DbusClient::GetPropOfContainer(const QString &userName, int32_t uid, const QString &prop)
{
    if (mKmreDBusInterface->isValid()) {
        QDBusMessage response = mKmreDBusInterface->call("GetPropOfContainer", userName, uid, prop);
        if (response.type() == QDBusMessage::ReplyMessage) {
            return response.arguments().takeFirst().toString();
        }
    }

    return QString();
}

// 从容器中的android环境获取默认属性
QString DbusClient::GetDefaultPropOfContainer(const QString &userName, int32_t uid, const QString &prop)
{
    if (mKmreDBusInterface->isValid()) {
        QDBusMessage response = mKmreDBusInterface->call("GetDefaultPropOfContainer", userName, uid, prop);
        if (response.type() == QDBusMessage::ReplyMessage) {
            return response.arguments().takeFirst().toString();
        }
    }

    return QString();
}

// 给容器中的android环境设置默认属性
void DbusClient::SetDefaultPropOfContainer(const QString &userName, int32_t uid, const QString &prop, const QString &value)
{
    if (mKmreDBusInterface->isValid()) {
        mKmreDBusInterface->call("SetDefaultPropOfContainer", userName, uid, prop, value);
    }
}

QString DbusClient::GetDisplayInformation()
{
    if (mManagerDBusInterface->isValid() && !KmreEnv::isKmreEnvStopped()) {
        QDBusMessage response = mManagerDBusInterface->call("getDisplayInformation");
        if (response.type() == QDBusMessage::ReplyMessage) {
            return response.arguments().takeFirst().toString();
        }
    }

    return QString();
}

bool DbusClient::isSessionDaemonRunning(const QString& name)
{
    QDBusConnection conn = QDBusConnection::sessionBus();
    if (conn.isConnected()) {
        QDBusReply<QString> reply = conn.interface()->call("GetNameOwner", name);
        return reply.value() != "";
    }

    return false;
}

void DbusClient::setCameraDevice(const QString &device)
{
    if (mManagerDBusInterface->isValid() && !KmreEnv::isKmreEnvStopped()) {
        mManagerDBusInterface->call("setCameraDevice", device);
    }
}

QString DbusClient::getCameraDevice()
{
    if (mManagerDBusInterface->isValid() && !KmreEnv::isKmreEnvStopped()) {
        QDBusMessage response = mManagerDBusInterface->call("getCameraDevice");
        if (response.type() == QDBusMessage::ReplyMessage) {
            return response.arguments().takeFirst().toString();
        }
    }

    return QString();
}

bool DbusClient::isHostSupportDDS()
{
    if (mManagerDBusInterface->isValid() && !KmreEnv::isKmreEnvStopped()) {
        QDBusMessage response = mManagerDBusInterface->call("isHostSupportDDS");
        if (response.type() == QDBusMessage::ReplyMessage) {
            return response.arguments().takeFirst().toBool();
        }
    }

    return false;
}

}
