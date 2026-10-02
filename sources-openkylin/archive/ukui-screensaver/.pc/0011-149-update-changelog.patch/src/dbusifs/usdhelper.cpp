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
#include "usdhelper.h"
#include <QCoreApplication>
#include <QDebug>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusConnectionInterface>

const static QString usdService = QStringLiteral("org.ukui.SettingsDaemon");
const static QString usdMediaKeysPath = QStringLiteral("/org/ukui/SettingsDaemon/MediaKeys");
const static QString usdMediaKeysInterface = QStringLiteral("org.ukui.SettingsDaemon.MediaKeys");

UsdHelper::UsdHelper(QObject *parent) : QObject(parent) {}

UsdHelper::~UsdHelper() {}

void UsdHelper::init()
{
    if (!usdInterface) {
        usdInterface = new QDBusInterface(
            usdService, usdMediaKeysPath, usdMediaKeysInterface, QDBusConnection::sessionBus(), this);
    }
}

bool UsdHelper::usdExternalDoAction(int actionType)
{
    init();
    QDBusPendingCall result = usdInterface->asyncCall("externalDoAction", actionType, "screensaver");
    if (result.reply().type() == QDBusMessage::ErrorMessage) {
        qWarning() << "usdExternalDoAction error!!!!!!!!!!!!!!";
        return false;
    }
    return true;
}
