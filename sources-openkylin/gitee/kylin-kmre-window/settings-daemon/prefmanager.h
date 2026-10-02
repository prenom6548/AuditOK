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

#ifndef PREFMANAGER_H
#define PREFMANAGER_H

#include <QObject>
#include <QDBusContext>

#define SERVICE_NAME "cn.kylinos.Kmre.Pref"
#define SERVICE_INTERFACE "cn.kylinos.Kmre.Pref"
#define SERVICE_PATH "/cn/kylinos/Kmre/Pref"

class PrefManager: public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", SERVICE_NAME)

public:
    explicit PrefManager(QObject *parent = nullptr);
    ~PrefManager();

public Q_SLOTS:
    bool setDockerDefaultIpAddress(const QString &value);
    bool addGameToWhiteList(const QString &appName, const QString &pkgName);
    bool removeGameFromWhiteList(const QString &pkgName);
    bool removeAppFromWarningList(const QString &pkgName);
    bool setKmreAutoStart(const QString &checked);
    bool setPhoneInfo(const QString &localpath, const QString &vendor, const QString &brand, const QString &name, const QString &model, const QString &equip, const QString &serialno, const QString &imei);
    int copyLogFiles(const QString &username, const QString &userid, const QString &logPath);
    void quilt();

private:
    bool checkEnviron(int pid);
    bool checkWhiteList(int pid, const QStringList& whiteList);
    bool isCallerAllowed(const QStringList& whiteList);

Q_SIGNALS:
    void aboutToQuit();
};

#endif // PREFMANAGER_H
