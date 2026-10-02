/*
 * Copyright 2025 KylinSoft Co., Ltd.
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

#ifndef DEVRPMINTERFACE_H
#define DEVRPMINTERFACE_H

#include <QObject>
#include <QVector>
#include <QMap>
#include <QSettings>
#include <QString>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include "devicepowerinfo.h"

#include "../upmcommonfile.h"
#include "../upmcustomhardwareconfig.h"

class DevRpmInterface
{
public:
    explicit DevRpmInterface();
    virtual ~DevRpmInterface();

protected:
    QVector<DevicePowerInfo *> m_devicePowerInfo;

    QString m_controlFile;
    QMap<QString, QString> m_powerConfigMap;

    //过滤不可配置的设备
    QMap<QString, QString> m_devFilterRulesMap;
    QMap<QString, QString> m_devIgnoreListMap;

    void getDevicePowerInfo();

public:
    void printConfigMap();
    int getDeviceNum();
    QString getDeviceName(int deviceIndex);
    QString getCurrentPowerStat(int deviceIndex);
    QString getDefaultPowerStat(int deviceIndex);

    virtual bool setPowerStat(int deviceIndex, const QString &stat) = 0;
    virtual bool setAllDevicePowerStat(const QString &stat) = 0;
    void printAllDeviceInfo();
    QMap<QString, QString> getPowerConfig();
    QString extractDeviceNameFromPath(const QString &devPath);
};

#endif
