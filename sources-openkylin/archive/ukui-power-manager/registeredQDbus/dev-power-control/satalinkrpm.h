/*
 * Copyright 2023 KylinSoft Co., Ltd.
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

#ifndef SATADEVICE_H
#define SATADEVICE_H

#include "devrpminterface.h"
#include "../upmcommonfile.h"
#include "../upmcustomhardwareconfig.h"

#define SATA_DEVICE_PATH "/sys/class/scsi_host/"

class SataLinkRpmDevice : public QObject, public DevRpmInterface
{
    Q_OBJECT
public:
    explicit SataLinkRpmDevice(QObject *parent = nullptr);

private:
    void getDevicePowerInfo();
    bool checkUsbSupportAutosuspend(const QString &devPath);
public:
    bool setPowerStat(int deviceIndex, const QString &stat);
    bool setAllDevicePowerStat(const QString &stat);
};

#endif // SATADEVICE_H
