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

#ifndef APPINFO_H
#define APPINFO_H

#include <QObject>
#include <QStringList>

#include <memory>

namespace kdk {

class AppInfoPrivate;

/**
 * @defgroup ApplicationsModule
 */

/**
 * @brief 应用信息类，解析存储应用信息，提供应用信息获取功能
 * @since 3.0.1.0
 */

class AppInfo : public QObject
{
    Q_OBJECT
public:
    explicit AppInfo(const QString &desktopFilePath, QObject *parent = nullptr);
    AppInfo(const AppInfo &other);

    virtual ~AppInfo();

public:
    QString name() const;
    QString type() const;
    QString exec() const;
    QString icon() const;
    QString comment() const;
    QString desktopName() const;
    QString desktopPath() const;
    QString genericName() const;
    QString terminalOptions() const;
    QStringList mimeTypes() const;
    bool terminal() const;
    bool isVaild() const;
    QVariant property(const QString &name) const;

private:
    Q_DECLARE_PRIVATE(AppInfo)
    std::unique_ptr<AppInfoPrivate> const d_ptr;
};

}

#endif // APPINFO_H
