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

#ifndef DBUS_CLIENT_H
#define DBUS_CLIENT_H

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusInterface>

#include <stdint.h>

namespace kmre {

class DBusClient
{
public:
    static DBusClient* getInstance();
    /* System bus method */
    int32_t Prepare(const QString& userName, int32_t uid);
    int32_t StartContainer(const QString& userName, int32_t uid, int32_t width, int32_t height);
    int32_t StartContainerSilently(const QString& userName, int32_t uid);
    int32_t ChangeContainerRuntimeStatus(const QString& userName, int32_t uid);
    //void SetFocusOnContainer(const QString& userName, int32_t uid, int32_t onFocus);
    QString GetPropOfContainer(const QString& userName, int32_t uid, const QString& prop);
    void SetPropOfContainer(const QString &userName, int32_t uid, const QString &prop, const QString &value);
    bool isHostSupportDDS();
    int32_t IsImageReady();
    void LoadImage();

    /* Session bus method */
    void StartKmreManager();
    void StartKmreWindow();
    bool GetDisplayInformation();
    void StartUserService(const QString &name, const QString &path, const QString &interface);
    QStringList GetRunningAppList();
    void LaunchApp(const QString& pkgName, int width, int height);
    void ActiveApp(const QString& pkgName);

private:
    DBusClient();
    ~DBusClient();
    static DBusClient* mDBusClient;
    QDBusInterface* mSystemBusInterface;
    QDBusInterface* mSessionBusInterface;
    QDBusInterface* mKmreWindowInterface;
};

} // namespace kmre

#endif // DBUS_CLIENT_H
