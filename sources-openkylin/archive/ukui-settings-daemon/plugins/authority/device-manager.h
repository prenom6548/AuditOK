/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2025 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QtDBus/QtDBus>

#include <libudev.h>
#include <QSocketNotifier>

#include "usd_base_class.h"

#define PROCBUSINPUTDEVICES "/proc/bus/input/devices"

#define DEVICEINFO_EV_KBD           "EV=120013"
#define DEVICEINFO_EV_MOUSE         "EV=17"
#define DEVICEINFO_EV_MOUSE_EX      "EV=1f"

#define DEVICEINFO_NAME_MOUSE       "Mouse"
#define DEVICEINFO_NAME_TOUCHPAD    "Touchpad"

struct DevInfo {
    QString name;
    QString phys;
    QString sysfs;
    QString ev;
};

typedef QList<DevInfo> DevInfoList;

class DeviceManager : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface",SETTINGS_DAEMON_SYSTEMDBUS_INTERFACE)
public:
    explicit DeviceManager(QObject *parent = nullptr);
    ~DeviceManager() override;

public Q_SLOTS:
    int GetDeviceNum(const QString &devType) const;

private:
    bool getDevInfoFromFile();
    void initUdevConnect();

    void initSignalTimer();

    void insertDevInfoIntoMap(const QString &id, const DevInfo &info);
    int getDevNumByType(const QString &devType) const;
    int getDevNumByName(const QString &devName) const;
    int getMouseNum() const;

private Q_SLOTS:
    void onFileChanged();

Q_SIGNALS:
    void DeviceNumChanged();

private:
    QMap<QString, DevInfoList> m_devInfoMap;

    QTimer* m_signalMergingTimer = nullptr;

    struct udev* m_udev = nullptr;
    struct udev_monitor *m_monitor = nullptr;
    QSocketNotifier* m_notifier = nullptr;
};

#endif // DEVICEMANAGER_H
