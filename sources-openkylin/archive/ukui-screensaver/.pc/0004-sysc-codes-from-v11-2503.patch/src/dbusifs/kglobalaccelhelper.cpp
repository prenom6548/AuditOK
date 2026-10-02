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
#include "kglobalaccelhelper.h"
#include <QCoreApplication>
#include <QDebug>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusConnectionInterface>

const static QString kglobalService = QStringLiteral("org.kde.kglobalaccel");
const static QString kglobalPath = QStringLiteral("/kglobalaccel");
const static QString kglobalInterface = QStringLiteral("org.kde.KGlobalAccel");

KglobalAccelHelper::KglobalAccelHelper(QObject *parent) : QObject(parent)
{
    m_kglobalInterface
        = new QDBusInterface(kglobalService, kglobalPath, kglobalInterface, QDBusConnection::sessionBus(), this);
}

KglobalAccelHelper::~KglobalAccelHelper() {}

bool KglobalAccelHelper::blockShortcut(bool val)
{
    QDBusMessage result = m_kglobalInterface->call("blockGlobalShortcuts", val);
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "blockShortcut error:" << result.errorMessage();
        return false;
    }
    return true;
}
