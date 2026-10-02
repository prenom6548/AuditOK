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

#ifndef WAKESOURCE_H
#define WAKESOURCE_H

#include "devrpminterface.h"

#define WAKE_SOURCE_PATH       "/sys/devices/"
//#define WAKE_SOURCE_FILE       "/device/power"

class WakeSourceRpmDevice : public QObject, public DevRpmInterface
{
    Q_OBJECT
public:
    explicit WakeSourceRpmDevice(QObject *parent = nullptr);

signals:

private:
    void getDevicePowerInfo();
    QString extractDevicePathFromWakeupFile(const QString &wakeupFilePath);
    bool isUsbDevice(const QString &devicePath);
    bool readFirstLine(const QString &filePath, QString &out);

public:
    bool setPowerStat(int deviceIndex, const QString &stat);
    bool setAllDevicePowerStat(const QString &stat);
};

#endif // WAKESOURCE_H
