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
#include "pcieaspm.h"

PcieAspmDevice::PcieAspmDevice(QObject *parent) : QObject(parent)
{
    m_powerConfigMap = UpmCustomHardwareConfig::self()->getConfig(UPM_GROUP_PCIEASPM);
    printConfigMap();
    m_controlFile = PCIE_ASPM_FILE;
    getDevicePowerInfo();
}

void PcieAspmDevice::getDevicePowerInfo()
{
    QDir deviceDir(PCIE_ASPM_PATH);
    if(!deviceDir.exists()) {
        qDebug() << "PcieAspmDevice::getPcieAspmDevicePowerInfo(): " << PCIE_ASPM_PATH << "not exists .";
        return;
    }

    const QString tempDevicePath = QString::fromStdString(PCIE_ASPM_PATH) + QString::fromStdString(PCIE_ASPM_FILE);
    QFileInfo checkFile(tempDevicePath);
    if(checkFile.exists() && checkFile.isFile()) {
        qDebug() << "PcieAspmDevice::getPcieAspmDevicePowerInfo(): " << "pcie_aspm device control file is exists: " << tempDevicePath;
        m_devicePowerInfo.push_back(new DevicePowerInfo("pcie_aspm", PCIE_ASPM_PATH, "pcie_aspm", m_controlFile, this));
    }
}

bool PcieAspmDevice::setPowerStat(int deviceIndex, const QString &stat)
{

    if (stat == "default" || stat == "performance" || stat == "powersave" || stat == "powersupersave") {
        if (deviceIndex < m_devicePowerInfo.count()) {
            return m_devicePowerInfo[deviceIndex]->setDevicePowerStat(stat);
        }
    }
    return false;
}

bool PcieAspmDevice::setAllDevicePowerStat(const QString &stat)
{
    qDebug() << "PcieAspmDevice::setAllDevicePowerStat: " << stat;
    if (stat == "default" || stat == "performance" || stat == "powersave" || stat == "powersupersave") {
        for (int i = 0; i < m_devicePowerInfo.count(); ++i) {
            m_devicePowerInfo[i]->setDevicePowerStat(stat);
        }
        return true;
    }
    return false;
}
