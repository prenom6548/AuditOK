/*
 * Copyright (C) 2023 KylinSoftCo., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/

#include "biodeviceinfo.h"

void registerCustomTypes()
{
    qDBusRegisterMetaType<DeviceInfo>();
    qDBusRegisterMetaType<QList<QDBusVariant> >();
}

QDBusArgument &operator<<(QDBusArgument &argument, const DeviceInfo &deviceInfo)
{
    argument.beginStructure();
    argument << deviceInfo.device_id << deviceInfo.device_shortname
        << deviceInfo.device_fullname << deviceInfo.driver_enable
        << deviceInfo.device_available
        << deviceInfo.biotype << deviceInfo.stotype
        << deviceInfo.eigtype << deviceInfo.vertype
        << deviceInfo.idtype << deviceInfo.bustype
        << deviceInfo.dev_status << deviceInfo.ops_status;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, DeviceInfo &deviceInfo)
{
    argument.beginStructure();
    argument >> deviceInfo.device_id >> deviceInfo.device_shortname
        >> deviceInfo.device_fullname >> deviceInfo.driver_enable
        >> deviceInfo.device_available
        >> deviceInfo.biotype >> deviceInfo.stotype
        >> deviceInfo.eigtype >> deviceInfo.vertype
        >> deviceInfo.idtype >> deviceInfo.bustype
        >> deviceInfo.dev_status >> deviceInfo.ops_status;
    argument.endStructure();
    return argument;
}
