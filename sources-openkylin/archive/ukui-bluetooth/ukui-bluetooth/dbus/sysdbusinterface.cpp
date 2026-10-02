/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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

#include "sysdbusinterface.h"

#include <QFileInfo>
#include <QDir>
#include <file-utils.h>
#include <QDBusInterface>

#include "ukui-log4qt.h"

sysdbusinterface::sysdbusinterface()
{

    QDBusConnection systemBus = QDBusConnection::systemBus();
    if (!systemBus.registerObject("/", this, QDBusConnection::ExportAllSlots))
    {
        KyWarning() << "QDbus register object failed reason:" << systemBus.lastError();
    }
}


sysdbusinterface::~sysdbusinterface()
{

}

QString sysdbusinterface::getProperFilePath(QString user, QString filename)
{
    KyInfo() << user << filename;
    QString dest;
    dest = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/" + filename;

    KyDebug() << dest;
    while (QFile::exists(dest))
    {
        QStringList newUrl = dest.split("/");
        newUrl.pop_back();
        newUrl.append(Peony::FileUtils::handleDuplicateName(Peony::FileUtils::urlDecode(dest)));
        dest = newUrl.join("/");
        KyDebug() << dest << newUrl;
    }

    return dest;
}

void sysdbusinterface::setMultiAudioCombine(bool v)
{
    KyInfo() << v;
    QDBusInterface iface("org.ukui.volume.control", "/org/ukui/volume/control",
                         "org.ukui.volume.control", QDBusConnection::sessionBus());
    iface.asyncCall("setMultiAudioCombine", 1, v, QStringList());
}



