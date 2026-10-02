/*
 * Copyright 2021 KylinSoft Co., Ltd.
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ENGINEDEVICE_H
#define ENGINEDEVICE_H

#include <QObject>
#include "device.h"
#include <QDBusMetaType>
#include <QDBusVariant>
#include <QString>
#include <QGSettings>

#define DBUS_SERVICE "org.freedesktop.UPower"
#define DBUS_OBJECT "/org/freedesktop/UPower"
#define DBUS_INTERFACE "org.freedesktop.UPower"

#define DBUS_INTERFACE_PRO "org.freedesktop.DBus.Properties"
#define DBUS_INTERFACE_DEV "org.freedesktop.UPower.Device"

class EngineDevice : public QObject
{
    Q_OBJECT
private:
    static EngineDevice *instance;
    explicit EngineDevice(QObject *parent = nullptr);
    class Deconstructor
    {
    public:
        ~Deconstructor()
        {
            if (instance) {
                delete instance;
                instance = nullptr;
            }
        }
    };
    static Deconstructor deconstructor;

public:
    static EngineDevice *getInstance()
    {
        if (instance == nullptr) {
            instance = new EngineDevice;
        }
        return instance;
    }

Q_SIGNALS:
    void engine_signal_discharge(DEV dv);
    void engine_signal_charge(DEV dv);
    void engine_signal_fullycharge(DEV dv);
    void engine_signal_charge_low(DEV dv);
    void engine_signal_charge_critical(DEV dv);
    void engine_signal_charge_action(DEV dv);
    void engine_signal_summary_change(QString summary);
    void engine_signal_Battery_State(QStringList Battery_State);
public Q_SLOTS:
    void power_device_change_callback(QDBusMessage msg, QString path);

public:
    QGSettings *settings;
    QList<DEVICE *> devices;
    QString previous_icon;
    QString previous_summary;

    DEVICE *composite_device;
    void power_device_recalculate_state();
    bool engine_recalculate_summary();
    void getProperty(QString path, DEV &dev);
    QString engine_get_summary();
    QStringList engine_get_state();
    QString engine_kind_to_localised_text(UpDeviceKind kind, uint number);
    void power_device_get_devices();

    QStringList engine_get_Battery_State(DEVICE *dv);

    QString boolToString(bool ret);
    QString engine_get_device_summary(DEVICE *dv);
    void putAttributes(QMap<QString, QVariant> &map, DEV &btrDetailData);
};

#endif // ENGINEDEVICE_H
