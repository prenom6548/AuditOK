/*
 * libkysdk-appmanager's Library
 *
 * Copyright (C) 2025, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: ShaoShuai Tian <tianshaoshuai@kylinos.cn>
 *
 */

#ifndef APPINFO_P_H
#define APPINFO_P_H
#include "appinfo.h"

#include <QMap>
#include <QVariant>
#include <QObject>

namespace kdk {

class AppInfoPrivate
{
public:
    explicit AppInfoPrivate(const QString &path);
    AppInfoPrivate(const AppInfoPrivate &other) = default;

    ~AppInfoPrivate();

public:
    void init(const QString &desktop, AppInfo *q);
    QVariant property(const QString &name, QMetaType t);

public:
    QStringList mCategories;
    QString mName;
    QString mExec;
    QString mType;
    QString mIcon;
    QString mTerminalOptions;
    QString mComment;
    QStringList mMimeTypes;
    QString mDesktopEntryPath;
    QString mDesktopEntryName;
    QString mWorkingDirectory;
    QMap<QString, QVariant> mMapProps;
    QString mGenericName;
    QString mUntranslatedName;
    QString mUntranslatedGenericName;

    bool mAllowAsDefault;
    bool mTerminal;
    bool mVaild;
};
}

#endif // APPINFO_P_H
