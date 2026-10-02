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

#include "acwatcher.h"
#include "dmi_chassis_type.h"

#include <ksoundeffects.h>
using namespace kdk;

AcWatcher::AcWatcher():mOnBattery(false)
{

}


void AcWatcher::initAcWatcher()
{
    if (hasBattery()) {
        QDBusConnection::systemBus().connect(
                    UPOWER_SERVICE, UPOWER_PATH, FREEDESKTOP_UPOWER, "PropertiesChanged", this, SLOT(dealAcWatcherMssage(void)));
        mOnBattery = readOnBattery();
    }
}

void AcWatcher::dealAcWatcherMssage()
{
    bool value;
    value = readOnBattery();
    if (mOnBattery != value) {
        mOnBattery = value;
        emit acChanged(mOnBattery);
        qDebug() << "Power status change signal transmission";

        if (false == mOnBattery) {
            KSoundEffects::playSound(POWER_PLUG);
        }
    }
}

bool AcWatcher::readOnBattery()
{
    QDBusInterface iface(UPOWER_SERVICE, UPOWER_PATH, FREEDESKTOP_UPOWER, QDBusConnection::systemBus());
    QDBusReply<QVariant> reply = iface.call("Get", "org.freedesktop.UPower", "OnBattery");
    if (reply.isValid()) {
        return reply.value().toBool();
    }
    qDebug() << "OnBattery error!";
    return false;
}

bool AcWatcher::hasBattery()
{
    QFile file("/sys/class/dmi/id/chassis_type");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open chassis_type file";
        return false;
    }

    QString chassisTypeStr = file.readAll().trimmed();
    file.close();

    bool ok = false;
    int chassisTypeInt = chassisTypeStr.toInt(&ok);
    if (!ok) {
        qWarning() << "Invalid chassis_type format:" << chassisTypeStr;
        return false;
    }

    switch (static_cast<DmiChassisType>(chassisTypeInt)) {
        case DmiChassisType::Laptop:
        case DmiChassisType::Notebook:
        case DmiChassisType::Tablet:
        case DmiChassisType::Convertible:
        case DmiChassisType::Detachable:
            return true;
        default:
            qDebug() << "Device type not supported for battery:" << chassisTypeInt;
            return false;
    }
}
