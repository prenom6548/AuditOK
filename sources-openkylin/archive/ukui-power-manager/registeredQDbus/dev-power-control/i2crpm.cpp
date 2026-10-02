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

#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include "i2crpm.h"

#include "../upmcommonfile.h"
#include "../upmcustomhardwareconfig.h"

I2cRpmDevice::I2cRpmDevice(QObject *parent) : QObject(parent)
{
    m_powerConfigMap = UpmCustomHardwareConfig::self()->getConfig(UPM_GROUP_I2CRPM);
    m_controlFile = QString("/power/control");

    getDevicePowerInfo();
}

void I2cRpmDevice::getDevicePowerInfo()
{
    QDir deviceDir(I2C_DEVICE_PATH);
    if(!deviceDir.exists()) {
        return;
    }

    QStringList deviceDirs = deviceDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    QString devicePath;
    for (int i = 0; i < deviceDirs.size(); ++i) {
        devicePath = I2C_DEVICE_PATH + deviceDirs.at(i);

        QString devName = getI2cRpmDeviceName(devicePath);

        QFile pmControlFile(devicePath + "/new_device");
        if (pmControlFile.exists()) {
            devicePath.append("/device");
        }

        if (true == deviceHasRuntimePM(devicePath)) {
            m_devicePowerInfo.push_back(new DevicePowerInfo("i2c", devicePath, devName, m_controlFile, this));
        }
    }
}

bool I2cRpmDevice::deviceHasRuntimePM(const QString &devPath)
{
    QFile pmFile(devPath + m_controlFile);
    if (!pmFile.exists()) {
        return false;
    }

    pmFile.setFileName(devPath + "/power/runtime_suspended_time");
    if (!pmFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCritical() << "open file error:" << pmFile.fileName();
        return false;
    }

    ulong value = pmFile.readLine().toULong();
    pmFile.close();
    if (value) {
        return true;
    }

    pmFile.setFileName(devPath + "/power/runtime_active_time");
    if (!pmFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCritical() << "open file error:" << pmFile.fileName();
        return false;
    }

    value = pmFile.readLine().toULong();
    pmFile.close();
    if (value) {
        return true;
    }

    return false;
}

QString I2cRpmDevice::getI2cRpmDeviceName(const QString &devPath)
{
    QString deviceName;
    QFile file(devPath + "/name");

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCritical() << "open file error:" << file.fileName();
        return "unknown";
    }

    deviceName = file.readLine();
    file.close();

    return deviceName.remove('\n');
}

bool I2cRpmDevice::setPowerStat(int deviceIndex, const QString &stat)
{
    if (stat == "auto" || stat == "on" || stat == "default") {
        if (deviceIndex < m_devicePowerInfo.count()) {
            return m_devicePowerInfo[deviceIndex]->setDevicePowerStat(stat);
        }
    }

    return false;
}

bool I2cRpmDevice::setAllDevicePowerStat(const QString &stat)
{
    qDebug() << "I2cRpmDevice::setAllDevicePowerStat: " << stat;
    if (stat == "auto" || stat == "on" || stat == "default") {
        for (int i = 0; i < m_devicePowerInfo.count(); ++i) {
            m_devicePowerInfo[i]->setDevicePowerStat(stat);
        }
        return true;
    }
    return false;
}
