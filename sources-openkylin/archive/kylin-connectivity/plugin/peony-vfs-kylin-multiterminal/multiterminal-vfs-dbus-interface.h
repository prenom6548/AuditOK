/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 * Authors: huheng <huheng@kylinos.cn>
 *
 */

#ifndef MULT_VFS_DBUS_INTERFACE_H
#define MULT_VFS_DBUS_INTERFACE_H

#include <QObject>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusInterface>
#include <QMutex>

#include "fileinfo.h"

#define ADDRESS_PREFIX "mult:///"

class MultVfsDbusInterface : public QObject
{
    Q_OBJECT
public:
    static MultVfsDbusInterface *getInstance();

public Q_SLOTS:
    void initDbusData();
    QString getMountDir();
    QString getDeviceName();
    QStringList getFileList(QString path);
    const FileInfo getFileInfo(QString filePath);
    void slotDeviceUpdate(QString deviceName);

Q_SIGNALS:
    void sigDeviceUpdate(QString);

    QString requestGetMountDir();
    QString requestGetDeviceName();
    QStringList requestGetFileList(QString path);
    FileInfo requestGetFileInfo(QString fileName);

private:
    explicit MultVfsDbusInterface(QObject *parent = nullptr);
    ~MultVfsDbusInterface();

    QMutex m_mutex;

    QDBusInterface *m_sessionBusInterface;
    QStringList m_fileList;
    QString m_mountDir = "";
    QString m_deviceName = "";
    QString m_oldPath = "";
};

#endif // MULT_VFS_DBUS_INTERFACE_H
