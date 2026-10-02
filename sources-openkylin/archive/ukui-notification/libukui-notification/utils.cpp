/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 *  This program is free software: you can redistribute it and/or modify
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
 *
 * Authors: iaom <zhangpengfei@kylinos.cn>
 */

#include "utils.h"
#include <QFile>
#include <QByteArray>
#include <QString>
#include <window-helper.h>

using namespace UkuiNotification;

UkuiSearch::ApplicationInfo &Utils::applicationInfo()
{
    static UkuiSearch::ApplicationInfo s_applicationInfo;
    return s_applicationInfo;
}

QString Utils::desktopEntryFromPid(uint pid, bool isFullPath)
{
    QString desktop;
    if(applicationInfo().tranPidToDesktopFp(pid, desktop)) {
        if (isFullPath) {
            return desktop;
        }
        return desktopEntry2Name(desktop);
    }
    return {};
}

QString Utils::displayFromPid(uint pid)
{
    QFile environFile(QStringLiteral("/proc/%1/environ").arg(QString::number(pid)));
    if (environFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        const QByteArray DISPLAY = UkuiQuick::WindowProxy::isWayland() ? QByteArrayLiteral("WAYLAND_DISPLAY") : QByteArrayLiteral("DISPLAY");
        const auto lines = environFile.readAll().split('\0');
        for (const QByteArray &line : lines) {
            const int equalsIdx = line.indexOf('=');
            if (equalsIdx <= 0) {
                continue;
            }
            const QByteArray key = line.left(equalsIdx);
            if (key == DISPLAY) {
                const QByteArray value = line.mid(equalsIdx + 1);
                return value;
            }
        }
    }
    return {};
}

QString Utils::desktopEntryFromName(QString desktopFileName)
{
    QString desktopFilePathName;
    applicationInfo().desktopFilePathFromName(desktopFileName, desktopFilePathName);
    return desktopFilePathName;
}

QString Utils::appNameForDesktopEntry(const QString &desktopFileName)
{
    auto appInfo = applicationInfo().getInfo(desktopFileName, UkuiSearch::ApplicationProperty::LocalName);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    if (appInfo.isValid() && appInfo.type() == QVariant::String) {
#else
    if (appInfo.isValid() && appInfo.typeId() == QVariant::String) {
#endif
        return appInfo.toString();
    }
    return {};
}