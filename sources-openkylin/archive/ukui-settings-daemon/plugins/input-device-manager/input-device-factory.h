/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2022 KylinSoft Co., Ltd.
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
 *
 * Authors: sundagao <sundagao@kylinos.cn>
 */
#ifndef INPUTDEVICEFACTORY_H
#define INPUTDEVICEFACTORY_H

#include <QObject>
#include <QDBusInterface>
#include <QThread>

#include "input-monitor.h"
#include "input-device-manager.h"

extern "C" {
#include <X11/extensions/XInput.h>
#include "clib-syslog.h"
}

struct WlcomDevice;
class InputDeviceManager;
class InputDeviceFactor : public QObject
{
    Q_OBJECT
public:
    virtual void initInputDevices() = 0;
    virtual void onCursorChanged(const QString&) = 0;
protected:
    InputDeviceManager* m_deviceManager = nullptr;
};

/*****X Device factor*****/

class InputXDeviceFactor : public InputDeviceFactor
{
    Q_OBJECT
public:
    explicit InputXDeviceFactor(QObject *parent = nullptr);
    ~InputXDeviceFactor();
    void initInputDevices() override;
private:
    void connectMonitor();
    InputDevice* createInputDevice(QVariant deviceId, DeviceType type, QString name);
    InputDevice* filterDevice(XDeviceInfo device);

private Q_SLOTS:
    void deviceAdd(int id);
    void deviceRemove(int id);
    void onCursorChanged(const QString&) override;
private:
    QThread   *m_monitorThread;
    InputMonitor* m_inputMonitor;
};

/*****wayland Device factor*****/

class InputWaylandDeviceFactor : public InputDeviceFactor
{
    Q_OBJECT
public:
    explicit InputWaylandDeviceFactor(QObject *parent = nullptr);
    ~InputWaylandDeviceFactor();
    void initInputDevices() override;
private:

    void connectMonitor();
    InputDevice* createInputDevice(QVariant deviceId, DeviceType type, QString name);
    InputDevice* filterDevice(QDBusInterface * interface);
    void managerAddDevice(QString);
private Q_SLOTS:
    void deviceAdd(QString device);
    void deviceRemove(QString device);
    void onCursorChanged(const QString&) override;
private:
    QDBusInterface* m_deviceInterface = nullptr;
};

/******wlcom input factor************/

class InputWlcomDeviceFactor: public InputDeviceFactor
{
  Q_OBJECT
public:
    explicit InputWlcomDeviceFactor(QObject* parent = nullptr);
    void initInputDevices() override;
private:
    InputDevice* filterDevice(int, uint, const WlcomDevice&);
    InputDevice* createInputDevice(const QVariant& deviceId, DeviceType type, const QVariant& productId, QString name);
    /**
     * @brief setCursor wlcom 光标大小设置
     * @param seat
     */
    void setCursor(const QString& seat);
private Q_SLOTS:
    void deviceAdd(const QString&, uint32_t);
    void deviceRemoved(const QString&);
    void onCursorChanged(const QString&) override;
private:
    QList<QString> m_seats;
};

/**********/

class InputDeviceFactorManager
{
public:
    static InputDeviceFactor* createDeviceFactor(InputDeviceManager* manager);
};



#endif // INPUTDEVICEFACTORY_H
