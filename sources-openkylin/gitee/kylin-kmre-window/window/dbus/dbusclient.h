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

#ifndef _DBUS_CLIENT_H
#define _DBUS_CLIENT_H

#include "utils.h"
#include "typedef.h"
#include "singleton.h"

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusInterface>
#include <QFileDialog>
#include <QThread>

namespace kmre {

class DbusClient : public QObject, public SingletonP<DbusClient>
{
    Q_OBJECT
public:
    /* For system bus. */
    int32_t Prepare(const QString& userName, int32_t uid);
    void StopContainer(const QString &userName, int uid);
    void SetPropOfContainer(const QString &userName, int32_t uid, const QString &prop, const QString &value);
    QString GetPropOfContainer(const QString& userName, int32_t uid, const QString& prop);
    QString GetDefaultPropOfContainer(const QString &userName, int32_t uid, const QString &prop);
    void SetDefaultPropOfContainer(const QString &userName, int32_t uid, const QString &prop, const QString &value);

    /* For session bus. */
    QString GetDisplayInformation();
    bool isHostSupportDDS();
    bool isSessionDaemonRunning(const QString& name);
    QString getCameraDevice();
    void setCameraDevice(const QString &device);

private:
    explicit DbusClient(QObject *parent = nullptr);
    ~DbusClient();

    up<QDBusInterface> mKmreDBusInterface;
    up<QDBusInterface> mManagerDBusInterface;

    friend SingletonP<DbusClient>;
};

} // namespace kmre

#endif // _DBUS_CLIENT_H
