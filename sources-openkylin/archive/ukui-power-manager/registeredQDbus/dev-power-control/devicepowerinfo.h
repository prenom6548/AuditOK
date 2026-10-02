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

#ifndef DEVICEPOWERINFO_H
#define DEVICEPOWERINFO_H

#include <QObject>
#include "../upmcommonfile.h"

class DevicePowerInfo : public QObject
{
    Q_OBJECT
public:
    explicit DevicePowerInfo(const QString &devType,
                             const QString &devPath,
                             const QString &devName,
                             const QString &controlFile,
                             QObject *parent = nullptr);

signals:

private:
    QString m_devType;
    QString m_devPath;
    QString m_controlFile;

    QString m_powerControlFilePath;

    QString m_devName;

    QString m_defaultPowerValue;
    QString m_currentPowerValue;

public:
    QString getDevType();
    QString getDevicePath();
    QString getDeviceName();
    QString getCurrentPowerStat();
    QString getDefaultPowerStat();

    void setDevName(const QString &devName);

    bool setDevicePowerStat(const QString &stat);
};

#endif // DEVICEPOWERINFO_H
