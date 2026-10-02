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

#include <QApplication>
#include <QTimer>
#include <QDebug>

#include "start_control.h"
#include "utils.h"
#include "get_userinfo.h"
#include "dbus_client.h"
#include "preinstallappmanager.h"

StartControl::StartControl(QObject *parent) : QObject(parent)
{
    QDBusConnection::systemBus().connect(QString("cn.kylinos.Kmre"),
                                         QString("/cn/kylinos/Kmre"),
                                         QString("cn.kylinos.Kmre"),
                                         QString("ImageLoaded"),
                                         this,
                                         SLOT(onImageLoaded()));
}

void StartControl::onImageLoaded()
{
    Utils::startContainerSilently();
#ifndef UKUI_WAYLAND
    PreInstallAppManager::waitForPreInstallFinished();
#endif
    onFinished();
}

void StartControl::onFinished()
{
    QApplication::exit();
}

void StartControl::startEnvSilently()
{
    if (kmre::DBusClient::getInstance()->IsImageReady()) {
        Utils::startContainerSilently();
#ifndef UKUI_WAYLAND
        PreInstallAppManager::waitForPreInstallFinished();
#endif
    }
    else {
        kmre::DBusClient::getInstance()->LoadImage();
    }

    QTimer *waitTimer = new QTimer(this);
    connect(waitTimer, &QTimer::timeout, this, [] {
        static int waitCount = 0;
        if (Utils::isAndroidDeskStart()) {
            syslog(LOG_INFO, "Kmre env started.");
            QApplication::exit(0);
        }
        else if (waitCount++ > 300) {
            syslog(LOG_ERR, "Starting Kmre env timeout!");
            QApplication::exit(-1);
        }
    });
    waitTimer->start(100);
}
