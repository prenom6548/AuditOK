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

QStringList list_printers_dev_path();

QString get_device_id_from_path(const QString &path);

#endif // UDEV_DEVICES_H
