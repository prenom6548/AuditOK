/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef DEVICEMONITOREVENTFILTER_H
#define DEVICEMONITOREVENTFILTER_H

#include <QAbstractNativeEventFilter>
#include <QObject>

class QTimer;
// If you are using multiple inheritance, moc assumes that the first inherited class is
// a subclass of QObject. Also, be sure that only the first inherited class is a QObject.
class DeviceMonitorEventFilter : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT
public:
    explicit DeviceMonitorEventFilter(QObject* parent = nullptr);
    ~DeviceMonitorEventFilter() override = default;

    bool nativeEventFilter(const QByteArray &eventType,
                           void* message, long int* result) override;
Q_SIGNALS:
    void touchScreenChanged(bool exist);
private:
    void setupTouchDevices();
    bool handleXcbGeEvent(void* message);

    // Filter recurring events for short periods of time(20ms)
    QTimer* m_timer = nullptr;
};

#endif // DEVICEMONITOREVENTFILTER_H
