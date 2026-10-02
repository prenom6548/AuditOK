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

#include "enginedevice.h"
#include "device.h"
#include <QDBusObjectPath>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDebug>

EngineDevice *EngineDevice::instance = NULL;

void EngineDevice::power_device_get_devices()
{
    QList<QDBusObjectPath> deviceNames;

    /* call enum dbus*/
    QDBusMessage msg = QDBusMessage::createMethodCall(DBUS_SERVICE, DBUS_OBJECT, DBUS_INTERFACE, "EnumerateDevices");
    QDBusMessage res = QDBusConnection::systemBus().call(msg);

    if (res.type() == QDBusMessage::ReplyMessage) {
        const QDBusArgument &dbusArg = res.arguments().at(0).value<QDBusArgument>();
        dbusArg >> deviceNames;
    } else {
    }
    int len = deviceNames.size();
    // qDebug()<<deviceNames.at(1).path();
    for (int i = 0; i < len; i++) {
        DEVICE *device = new DEVICE(this);
        device->m_dev.path = deviceNames.at(i).path();
        getProperty(device->m_dev.path, device->m_dev);
        /* connect the nofity signal to changecallback */
        QDBusConnection::systemBus().connect(
            DBUS_SERVICE,
            device->m_dev.path,
            DBUS_INTERFACE_PRO,
            QString("PropertiesChanged"),
            device,
            SLOT(handleChanged(QDBusMessage)));
        connect(
            device,
            SIGNAL(device_property_changed(QDBusMessage, QString)),
            this,
            SLOT(power_device_change_callback(QDBusMessage, QString)));

        /* add to device list*/
        devices.append(device);
    }
}

EngineDevice::EngineDevice(QObject *parent) : QObject(parent)
{
    if (!settings && QGSettings::isSchemaInstalled(GPM_SETTINGS_SCHEMA)) {
        settings = new QGSettings(GPM_SETTINGS_SCHEMA);
    }
    power_device_get_devices();
}

void EngineDevice::getProperty(QString path, DEV &dev)
{
    QDBusMessage msg = QDBusMessage::createMethodCall(DBUS_SERVICE, path, DBUS_INTERFACE_PRO, "GetAll");
    msg << DBUS_INTERFACE_DEV;
    QDBusMessage res = QDBusConnection::systemBus().call(msg);

    if (res.type() == QDBusMessage::ReplyMessage) {
        const QDBusArgument &dbusArg = res.arguments().at(0).value<QDBusArgument>();
        QMap<QString, QVariant> map;
        dbusArg >> map;

        dev.kind = (UpDeviceKind)map.value(QString("Type")).toInt();
        dev.Type = engine_kind_to_localised_text((UpDeviceKind)map.value(QString("Type")).toInt(), 1);
        dev.Model = map.value(QString("Model")).toString();
        dev.Device = map.value(QString("NativePath")).toString();
        dev.IsPresent = (map.value(QString("IsPresent")).toBool());
        dev.PowerSupply = boolToString(map.value(QString("PowerSupply")).toBool());
        dev.Percentage = map.value(QString("Percentage")).toDouble();
        dev.Percentage = ((float)((int)((dev.Percentage + 0.05) * 10))) / 10;

        dev.Online = boolToString(map.value(QString("Online")).toBool());
        dev.State = (UpDeviceState)map.value(QString("State")).toInt();
        dev.TimeToEmpty = map.value(QString("TimeToEmpty")).toLongLong();
        dev.TimeToFull = map.value(QString("TimeToFull")).toLongLong();
    }
}

QString EngineDevice::boolToString(bool ret)
{
    return ret ? tr("yes") : tr("no");
}

void EngineDevice::putAttributes(QMap<QString, QVariant> &map, DEV &btrDetailData)
{
    if (map.contains("TimeToFull")) {
        btrDetailData.TimeToFull = map.value(QString("TimeToFull")).toLongLong();
    }
    if (map.contains("TimeToEmpty")) {
        btrDetailData.TimeToEmpty = map.value(QString("TimeToEmpty")).toLongLong();
    }
    if (map.contains("State")) {
        btrDetailData.State = (UpDeviceState)map.value(QString("State")).toInt();
    }
    if (map.contains("Percentage")) {
        btrDetailData.Percentage = map.value(QString("Percentage")).toDouble();
        btrDetailData.Percentage = ((float)((int)((btrDetailData.Percentage + 0.05) * 10))) / 10;
    }
    if (map.contains("PowerSupply")) {
        btrDetailData.PowerSupply = (map.value(QString("PowerSupply")).toBool()) ? tr("Yes") : tr("No");
    }
    if (map.contains("IsPresent")) {
        btrDetailData.IsPresent = (map.value(QString("IsPresent")).toBool());
    }
}

void EngineDevice::power_device_change_callback(QDBusMessage msg, QString path)
{
    /* if battery change to display devices */
    /* judge state */
    DEVICE *item = nullptr;
    Q_FOREACH (auto item_tmp, devices) {
        if (item_tmp->m_dev.path == path) {
            item = item_tmp;
            break;
        }
    }

    if (item == nullptr)
        return;
    const QDBusArgument &arg = msg.arguments().at(1).value<QDBusArgument>();
    QMap<QString, QVariant> map;
    arg >> map;
    putAttributes(map, item->m_dev);

    /*recaculate state*/
    power_device_recalculate_state();
}

void EngineDevice::power_device_recalculate_state()
{
    engine_recalculate_summary();
}

/**
 * engine_recalculate_summary:
 */
bool EngineDevice::engine_recalculate_summary()
{
    QString summary;
    QStringList Battery_State;

    Battery_State = engine_get_state();
    summary = engine_get_summary();
    if (Battery_State.isEmpty()) {
        return false;
    }
    if (previous_summary.isNull()) {
        previous_summary = summary;
        Q_EMIT engine_signal_summary_change(summary);
        Q_EMIT engine_signal_Battery_State(Battery_State);
        return true;
    }

    if (previous_summary != summary) {
        previous_summary = summary;
        Q_EMIT engine_signal_summary_change(summary);
        Q_EMIT engine_signal_Battery_State(Battery_State);
        return true;
    }
    return false;
}

QStringList EngineDevice::engine_get_state()
{
    DEVICE *device;
    UpDeviceState state;
    QStringList tooltip;
    QStringList part;
    bool is_present;
    UpDeviceKind kind;

    Q_FOREACH (device, devices) {
        is_present = device->m_dev.IsPresent;
        state = device->m_dev.State;
        kind = device->m_dev.kind;
        if ((!is_present) || (kind != UP_DEVICE_KIND_BATTERY))
            continue;
        if (state == UP_DEVICE_STATE_EMPTY)
            continue;
        part = engine_get_Battery_State(device);
        if (!part.isEmpty())
            tooltip.append(part);
    }
    return tooltip;
}

QStringList EngineDevice::engine_get_Battery_State(DEVICE *dv)
{
    UpDeviceState state;
    double percentage;
    QStringList result;
    state = dv->m_dev.State;
    int EMPTY = dv->m_dev.TimeToEmpty;
    percentage = dv->m_dev.Percentage;
    bool is_present;
    is_present = dv->m_dev.IsPresent;
    if (!is_present)
        return result;
    result.append(QString("%1").arg(percentage));
    result.append(QString("%1").arg(state));
    result.append(QString("%1").arg(EMPTY));
    return result;
}

/**
 * engine_get_summary:
 *
 * Returns the complete tooltip ready for display
 **/
QString EngineDevice::engine_get_summary()
{
    DEVICE *device;
    UpDeviceState state;
    QString tooltip;
    QString part;
    bool is_present;
    UpDeviceKind kind;

    Q_FOREACH (device, devices) {
        is_present = device->m_dev.IsPresent;
        state = device->m_dev.State;
        kind = device->m_dev.kind;
        if ((!is_present) || (kind != UP_DEVICE_KIND_BATTERY))
            continue;
        if (state == UP_DEVICE_STATE_EMPTY)
            continue;
        part = engine_get_device_summary(device);
        if (!part.isNull())
            tooltip = QString("%1").arg(part);
    }
    return tooltip;
}

/**
 * engine_get_device_summary:
 **/

QString EngineDevice::engine_get_device_summary(DEVICE *dv)
{
    QString kind_desc;
    UpDeviceKind kind;
    UpDeviceState state;
    double percentage;
    bool is_present;
    uint time_to_full;
    uint time_to_empty;

    QString result;

    kind = dv->m_dev.kind;
    is_present = dv->m_dev.IsPresent;
    state = dv->m_dev.State;
    percentage = dv->m_dev.Percentage;
    time_to_empty = dv->m_dev.TimeToEmpty;
    time_to_full = dv->m_dev.TimeToFull;
    if (!is_present)
        return NULL;

    kind_desc = engine_kind_to_localised_text(kind, 1);

    if (state == UP_DEVICE_STATE_FULLY_CHARGED) {

        result = tr("%1% available, charged").arg(percentage);

    } else if (state == UP_DEVICE_STATE_DISCHARGING) {

        int is_show = 0;
        if (settings) {
            is_show = settings->get(GPM_SETTINGS_DISPLAY_LEFT_TIME).toInt();
        }
        if (is_show) {
            result = tr("Left %1h %2m (%3%)")
                         .arg((time_to_empty) / 3600)
                         .arg(((time_to_empty) % 3600) / 60)
                         .arg(percentage);
        } else {
            result = tr("%1% available").arg(percentage);
        }

    } else if (state == UP_DEVICE_STATE_CHARGING) {
        int is_show = 0;
        if (settings) {
            is_show = settings->get(GPM_SETTINGS_DISPLAY_LEFT_TIME).toInt();
        }
        if (is_show) {
            result = tr("Left %1h %2m to full").arg((time_to_full) / 3600).arg(((time_to_full) % 3600) / 60);
        } else {
            result = tr("charging (%1%)").arg(percentage);
        }

    } else if (state == UP_DEVICE_STATE_PENDING_DISCHARGE) {

        /* TRANSLATORS: this is only shown for laptops with multiple batteries */
        result = tr("%1 waiting to discharge (%2%)").arg(kind_desc).arg(percentage);

    } else if (state == UP_DEVICE_STATE_PENDING_CHARGE) {

        /* TRANSLATORS: this is only shown for laptops with multiple batteries */
        result = tr("%1 waiting to charge (%2%)").arg(kind_desc).arg(percentage);

    } else {
        printf("in an undefined state we are not charging or "
               "discharging and the batteries are also not charged");
        result = QString("%1 (%2%)").arg(kind_desc).arg(percentage);
    }
    return result;
}

/**
 * engine_kind_to_localised_text:
 **/
QString EngineDevice::engine_kind_to_localised_text(UpDeviceKind kind, uint number)
{
    Q_UNUSED(number);
    QString text;
    switch (kind) {
        case UP_DEVICE_KIND_LINE_POWER:

            text = tr("AC adapter");
            break;
        case UP_DEVICE_KIND_BATTERY:
            /* TRANSLATORS: laptop primary battery */
            text = tr("Laptop battery");
            break;
        case UP_DEVICE_KIND_UPS:
            /* TRANSLATORS: battery-backed AC power source */
            text = tr("UPS");
            break;
        case UP_DEVICE_KIND_MONITOR:
            /* TRANSLATORS: a monitor is a device to measure voltage and current */
            text = tr("Monitor");
            break;
        case UP_DEVICE_KIND_MOUSE:
            /* TRANSLATORS: wireless mice with internal batteries */
            text = tr("Mouse");
            break;
        case UP_DEVICE_KIND_KEYBOARD:
            /* TRANSLATORS: wireless keyboard with internal battery */
            text = tr("Keyboard");
            break;
        case UP_DEVICE_KIND_PDA:
            /* TRANSLATORS: portable device */
            text = tr("PDA");
            break;
        case UP_DEVICE_KIND_PHONE:
            /* TRANSLATORS: cell phone (mobile...) */
            text = tr("Cell phone");
            break;
        case UP_DEVICE_KIND_MEDIA_PLAYER:
            /* TRANSLATORS: media player, mp3 etc */
            text = tr("Media player");
            break;
        case UP_DEVICE_KIND_TABLET:
            /* TRANSLATORS: tablet device */
            text = tr("Tablet");
            break;
        case UP_DEVICE_KIND_COMPUTER:
            /* TRANSLATORS: tablet device */
            text = tr("Computer");
            break;
        default:
            printf("enum unrecognised: %i", kind);
            text = tr("unrecognised");
    }
    return text;
}
