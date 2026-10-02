/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
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
 *
 */

#include "icon-loader.h"
#include <QMutex>
#include <QMutexLocker>
#include <XdgIcon>
#include <QFile>
#include <QFileInfo>

namespace UkuiSearch {
static QMutex qtIconMutex;
static QMutex xdgIconMutex;
QIcon IconLoader::loadIconQt(const QString &name, const QIcon &fallback)
{
    QMutexLocker locker(&qtIconMutex);
    const QFileInfo info(name);

    if (!info.exists()) {
        if (name.isEmpty() || name.startsWith("/")) {
            return fallback;
        }
        return QIcon::fromTheme(name, fallback);
    }

    if (!info.isReadable()) {
        return fallback;
    }

    QIcon icon(name);
    if (!icon.isNull()) {
        return icon;
    }
    return fallback;
}

QIcon IconLoader::loadIconXdg(const QString &name, const QIcon &fallback)
{
    QMutexLocker locker(&xdgIconMutex);
    const QFileInfo info(name);

    if (!info.exists()) {
        if (name.isEmpty() || name.startsWith("/")) {
            return fallback;
        }
    } else {
        if (!info.isReadable()) {
            return fallback;
        }
    }

    return XdgIcon::fromTheme(name,fallback);
}
} // UkuiSearch