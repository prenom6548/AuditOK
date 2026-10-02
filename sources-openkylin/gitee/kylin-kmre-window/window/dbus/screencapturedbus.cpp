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

#include "screencapturedbus.h"
#include "syslog.h"

#include <QDBusReply>

namespace kmre {

ScreenCaptureDbus::ScreenCaptureDbus()
    : mDbusThread(new QThread())
{
    this->moveToThread(mDbusThread);
    connect(mDbusThread, SIGNAL(started()), this, SLOT(onInit()), Qt::QueuedConnection);
    connect(mDbusThread, &QThread::finished, mDbusThread, &QThread::deleteLater);
    mDbusThread->start();
}

ScreenCaptureDbus::~ScreenCaptureDbus()
{
    mDbusThread->quit();
    mDbusThread->wait();
}

void ScreenCaptureDbus::onInit()
{
    // 屏幕截图接口
    if (!mScreenShotDbusKylinInterface) {
        mScreenShotDbusKylinInterface = std::make_unique<QDBusInterface>("org.dharkael.kylinscreenshot",
                                                                        "/",
                                                                        "org.dharkael.kylinscreenshot",
                                                                        QDBusConnection::sessionBus());
        
        if (mScreenShotDbusKylinInterface->isValid()) {
            connect(mScreenShotDbusKylinInterface.get(), SIGNAL(captureTaken(uint, QByteArray)), this, SIGNAL(sigCaptureTaken(uint, QByteArray)));
            connect(mScreenShotDbusKylinInterface.get(), SIGNAL(captureExit(uint)), this, SIGNAL(sigCaptureExit(uint)));
            connect(mScreenShotDbusKylinInterface.get(), SIGNAL(captureCopy(uint)), this, SIGNAL(sigCaptureCopy(uint)));
            connect(mScreenShotDbusKylinInterface.get(), SIGNAL(captureFailed(uint)), this, SIGNAL(sigCaptureFailed(uint)));
        }
        else {
            syslog(LOG_ERR, "[%s] Screenshot dbus of kylin interface is invalid!", __func__);
        }
    }

    if (!mScreenShotDbusIntrospectable) {
        mScreenShotDbusIntrospectable = std::make_unique<QDBusInterface>("org.dharkael.kylinscreenshot", 
                                                                        "/", 
                                                                        "org.freedesktop.DBus.Introspectable", 
                                                                        QDBusConnection::sessionBus());

        if (!mScreenShotDbusIntrospectable->isValid()) {
            syslog(LOG_ERR, "[%s] Screenshot dbus of Introspectable interface is invalid!", __func__);
        }
    }

    syslog(LOG_INFO, "[%s] Screen capture dbus interface inited.", __func__);
}

// dbus-monitor interface=org.dharkael.kylinscreenshot
// dbus-send --session --type=signal / org.dharkael.kylinscreenshot.captureExit uint32:1
// dbus-send --session --dest=org.dharkael.kylinscreenshot --type=method_call --print-reply / org.dharkael.kylinscreenshot.graphicCapture string:"~/图片" int32:0 uint32:1

void ScreenCaptureDbus::graphicCapture(const QString& path, int delay, uint id)
{
    syslog(LOG_DEBUG, "[%s] Call 'graphicCapture' interface...", __func__);
    if (mScreenShotDbusKylinInterface && mScreenShotDbusKylinInterface->isValid()) {
        mScreenShotDbusKylinInterface->call("graphicCapture", path, delay, id);
    }
    else {
        syslog(LOG_ERR, "[%s] Dbus 'KylinInterface' is not ready!", __func__);
    }
}

QString ScreenCaptureDbus::introspect()
{
    syslog(LOG_DEBUG, "[%s] Call 'Introspect' interface...", __func__);
    if (mScreenShotDbusIntrospectable && mScreenShotDbusIntrospectable->isValid()) {
        QDBusReply<QString> reply = mScreenShotDbusIntrospectable->call("Introspect");
        if (reply.isValid()) {
            return reply.value();
        }
        else {
            syslog(LOG_ERR, "[%s] Call 'Introspect' interface failed!", __func__);
        }
    }
    else {
        syslog(LOG_ERR, "[%s] Dbus 'Introspectable' is not ready!", __func__);
    }

    return "";
}

QVariantMap ScreenCaptureDbus::getSaveInfo()
{
    if (mScreenShotDbusKylinInterface && mScreenShotDbusKylinInterface->isValid()) {
        QDBusReply<QVariantMap> reply = mScreenShotDbusKylinInterface->call("getSaveInfo");
        if (reply.isValid()) {
            return reply.value();
        }
        else {
            syslog(LOG_ERR, "[%s] Call 'getSaveInfo' interface failed!", __func__);
        }
    }
    else {
        syslog(LOG_ERR, "[%s] Dbus 'KylinInterface' is not ready!", __func__);
    }

    return QVariantMap();
}

}