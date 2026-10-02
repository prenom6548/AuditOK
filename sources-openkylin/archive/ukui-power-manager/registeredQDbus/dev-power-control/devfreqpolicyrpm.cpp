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
#include "devfreqpolicyrpm.h"

DevFreqPolicyRpm::DevFreqPolicyRpm(QObject *parent) : QObject(parent)
{
    m_powerConfigMap = UpmCustomHardwareConfig::self()->getConfig(UPM_GROUP_DEVFREQPOLICY);
    printConfigMap();
    m_controlFile = DEV_FREQ_FILE;
    getDevicePowerInfo();
}

void DevFreqPolicyRpm::getDevicePowerInfo()
{/*
    QDir deviceDir(NET_WOL_PATH);
    if(!deviceDir.exists()) {
        return;
    }

    QStringList deviceDirs = deviceDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (int i = 0; i < deviceDirs.size(); ++i) {
        QString devicePath = NET_WOL_PATH + deviceDirs.at(i);
        if (!devicePath.isEmpty() && devicePath.back() != '/') {
            devicePath += "/";
        }

        const QString tempDevicePath = devicePath + QString::fromStdString(NET_WOL_FILE);
        QFileInfo checkFile(tempDevicePath);
        if(checkFile.exists() && checkFile.isFile()) {
            QString devName = deviceDirs.at(i);
            qDebug() << "PcieAspmDevice::getPcieAspmDevicePowerInfo(): " << "pcie_aspm device control file is exists: " << tempDevicePath;
            m_devicePowerInfo.push_back(new DevicePowerInfo("net_wol", devicePath, "net_wol", m_controlFile, this));
        }


    }*/
    QDir deviceDir(DEV_FREQ_PATH);
    if(!deviceDir.exists()) {
        return;
    }

    QStringList deviceDirs = deviceDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (int i = 0; i < deviceDirs.size(); ++i) {
        QString devicePath = DEV_FREQ_PATH + deviceDirs.at(i);
        if (!devicePath.isEmpty() && devicePath.back() != '/') {
            devicePath += "/";
        }

        const QString tempDevicePath = devicePath + QString::fromStdString(DEV_FREQ_FILE);
        QFileInfo checkFile(tempDevicePath);
        if(checkFile.exists() && checkFile.isFile()) {
            QString devName = deviceDirs.at(i);
            qDebug() << "DevFreqPolicyRpm::getDevicePowerInfo(): " << "dev_freq device control file is exists: " << tempDevicePath;
            m_devicePowerInfo.push_back(new DevicePowerInfo("dev_freq", devicePath, devName, m_controlFile, this));
        }
    }
}

bool DevFreqPolicyRpm::setPowerStat(int deviceIndex, const QString &stat)
{

    if (stat == "powersave" || stat == "simple_ondemand") {
        if (deviceIndex < m_devicePowerInfo.count()) {
            return m_devicePowerInfo[deviceIndex]->setDevicePowerStat(stat);
        }
    }
    return false;
}

bool DevFreqPolicyRpm::setAllDevicePowerStat(const QString &stat)
{
    qDebug() << "DevFreqPolicyRpm::setAllDevicePowerStat: " << stat;
    if (stat == "powersave" || stat == "simple_ondemand" || stat == "performance" || stat == "userspace") {
        for (int i = 0; i < m_devicePowerInfo.count(); ++i) {
            m_devicePowerInfo[i]->setDevicePowerStat(stat);
        }
        return true;
    }
    return false;
}
