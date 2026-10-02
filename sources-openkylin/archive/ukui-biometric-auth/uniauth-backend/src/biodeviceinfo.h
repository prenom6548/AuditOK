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
#ifndef BIODEVICEINFO_H
#define BIODEVICEINFO_H

#include <QDBusInterface>
#include <QDBusArgument>
#include <QDBusMetaType>
#include <memory>

struct DeviceInfo {
    int device_id;
    QString device_shortname; /* aka driverName */
    QString device_fullname;
    int driver_enable; /* The corresponding driver is enabled/disabled */
    int device_available; /* The driver is enabled and the device is connected */
    int biotype;
    int stotype;
    int eigtype;
    int vertype;
    int idtype;
    int bustype;
    int dev_status;
    int ops_status;
};

enum BioType {
    BIOTYPE_FINGERPRINT,
    BIOTYPE_FINGERVEIN,
    BIOTYPE_IRIS,
    BIOTYPE_FACE,
    BIOTYPE_VOICEPRINT,
    __MAX_NR_BIOTYPES
};

#define UNIT_GENERAL_UKEY   (6)
#define REMOTE_QRCODE_TYPE  (8)

Q_DECLARE_METATYPE(DeviceInfo)
Q_DECLARE_METATYPE(QList<QDBusVariant>)

typedef std::shared_ptr<DeviceInfo> DeviceInfoPtr;
typedef QList<DeviceInfoPtr> DeviceList;

void registerCustomTypes();
QDBusArgument &operator<<(QDBusArgument &argument, const DeviceInfo &deviceInfo);
const QDBusArgument &operator>>(const QDBusArgument &argument, DeviceInfo &deviceInfo);

#endif // BIODEVICEINFO_H
