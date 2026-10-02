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
#ifndef UDEV_DEVICES_H
#define UDEV_DEVICES_H

#include "device_information.h"

struct device_id_s {
    QString full_device_id;
    QString mfg;
    QString mdl;
    QString sern;
};

void
parse_device_id (const char *device_id, struct device_id_s *id);

void deviceIdToDeviceInfo(DeviceInformation &info);

QString find_matching_device_uri (const QString &deviceId);

QStringList list_printers_dev_path();

QString get_device_id_from_path(const QString &path);

QString get_device_id_from_path_retry(const QString &path);

#endif // UDEV_DEVICES_H
