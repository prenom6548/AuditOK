/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 */

#pragma once

#include <QObject>
#include <QSocketNotifier>
#include <QVector>
#include <QMap>
#include <libusb-1.0/libusb.h>

/**
 * @brief 监听 USB 热插拔事件的类
 * 使用 libusb_hotplug_register_callback 与 QSocketNotifier 结合 Qt 事件循环
 */
class QUsbHotplugWatcher : public QObject
{
    Q_OBJECT
public:
    explicit QUsbHotplugWatcher(QObject *parent = nullptr);
    ~QUsbHotplugWatcher();

signals:
    void deviceAdded(quint16 vid, quint16 pid);
    void deviceRemoved(quint16 vid, quint16 pid);

private:
    static int hotplugCallback(libusb_context *ctx,
                               libusb_device *dev,
                               libusb_hotplug_event event,
                               void *user_data);

    void setupNotifiers();
    void cleanup();

    libusb_context *m_ctx = nullptr;
    libusb_hotplug_callback_handle m_cbHandle;
    QVector<QSocketNotifier*> m_notifiers;
};
