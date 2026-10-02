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
#include <QDebug>

#include "satalinkrpm.h"

SataLinkRpmDevice::SataLinkRpmDevice(QObject *parent) : QObject(parent)
{
    m_powerConfigMap = UpmCustomHardwareConfig::self()->getConfig(UPM_GROUP_SATALINKPOWER);
    m_controlFile = QString("/link_power_management_policy");

    getDevicePowerInfo();
}

void SataLinkRpmDevice::getDevicePowerInfo()
{
    QDir deviceDir(SATA_DEVICE_PATH);
    if(!deviceDir.exists()) {
        return;
    }

    QStringList deviceDirs = deviceDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    QString devicePath;
    QFile pmControlFile;
    for (int i = 0; i < deviceDirs.size(); ++i) {
        devicePath = SATA_DEVICE_PATH + deviceDirs.at(i);

        pmControlFile.setFileName(devicePath + m_controlFile);
        if (!pmControlFile.exists()) {
            continue;
        }
        QString devName = QString("STAT link PM " + deviceDirs.at(i));
        m_devicePowerInfo.push_back(new DevicePowerInfo("sata", devicePath, devName, m_controlFile, this));
    }
}

bool SataLinkRpmDevice::setPowerStat(int deviceIndex, const QString &stat)
{
    if (stat == "min_power" || stat == "max_performance" || stat == "default" || stat == "medium_power") {
        if (deviceIndex < m_devicePowerInfo.count()) {
            return m_devicePowerInfo[deviceIndex]->setDevicePowerStat(stat);
        }
    }

    return false;
}

bool SataLinkRpmDevice::setAllDevicePowerStat(const QString &stat)
{
    qDebug() << "SataLinkRpmDevice: Setting all device to power stat:" << stat;
    if (stat == "min_power" || stat == "max_performance" || stat == "default" || stat == "medium_power") {
        for (int i = 0; i < m_devicePowerInfo.count(); ++i) {
            m_devicePowerInfo[i]->setDevicePowerStat(stat);
        }
        return true;
    }
    return false;
}
