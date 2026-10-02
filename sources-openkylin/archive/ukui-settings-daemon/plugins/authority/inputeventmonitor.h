/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2026 KylinSoft Co., Ltd.
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

#ifndef INPUTEVENTMONITOR_H
#define INPUTEVENTMONITOR_H

#include <QObject>
#include <QThread>
#include <vector>
#include <string>
#include <atomic>
#include <sys/inotify.h>

#include "usd_global_define.h"
#include "clib-syslog.h"

struct libevdev;

class InputMonitor : public QObject {
    Q_OBJECT

public:
    explicit InputMonitor(QObject *parent = nullptr);
    ~InputMonitor() override;

public Q_SLOTS:
    void startMonitoring();
    void stopMonitoring();

Q_SIGNALS:
    void mouseMoved(int dx, int dy);

private:
    struct DeviceContext {
        struct libevdev *dev = nullptr;
        int fd = -1;
        bool isTouch = false;
        int lastAbsX = -1;
        int lastAbsY = -1;
        std::string path;

        DeviceContext() = default;
        DeviceContext(const DeviceContext&) = delete;
        DeviceContext& operator=(const DeviceContext&) = delete;
        DeviceContext(DeviceContext&&) = default;
        DeviceContext& operator=(DeviceContext&&) = default;
    };

    std::vector<DeviceContext> m_devices;
    int m_inotifyFd = -1;
    int m_watchFd = -1;
    std::atomic<bool> m_running{false};

    void initInotify();
    void handleInotifyEvents();
    void addDevice(const std::string& path);
    void removeDeviceByPath(const std::string& path);
    void removeDeviceByIndex(size_t index);
    void cleanupAllDevices();
    void scanInitialDevices();
};

class InputEventMonitor : public QObject {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", SETTINGS_DAEMON_SYSTEMDBUS_INTERFACE)

public:
    explicit InputEventMonitor(QObject *parent = nullptr);
    ~InputEventMonitor() override;

Q_SIGNALS:
    void MouseMoved(int dx, int dy);

private:
    QThread *m_workerThread = nullptr;
    InputMonitor *m_monitor = nullptr;
};

#endif // INPUTEVENTMONITOR_H