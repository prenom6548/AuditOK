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
#include "audiorpm.h"

IntelAudioRpmDevice::IntelAudioRpmDevice(QObject *parent) : QObject(parent)
{
    m_powerConfigMap = UpmCustomHardwareConfig::self()->getConfig(UPM_GROUP_SNAHDAINTELAUDIO);
    printConfigMap();
    m_controlFile = INTEL_AUDIO_FILE;
    getDevicePowerInfo();
}

void IntelAudioRpmDevice::getDevicePowerInfo()
{
    QDir deviceDir(INTEL_AUDIO_PATH);
    if(!deviceDir.exists()) {
        return;
    }

    const QString tempDevicePath = QString::fromStdString(INTEL_AUDIO_PATH) + QString::fromStdString(INTEL_AUDIO_FILE);
    QFileInfo checkFile(tempDevicePath);
    if(checkFile.exists() && checkFile.isFile()) {
        m_devicePowerInfo.push_back(new DevicePowerInfo("intel_audio", INTEL_AUDIO_PATH, "intel_audio", m_controlFile, this));
    }
}

bool IntelAudioRpmDevice::setPowerStat(int deviceIndex, const QString &stat)
{

    if (stat == "0" || stat == "1") {
        if (deviceIndex < m_devicePowerInfo.count()) {
            return m_devicePowerInfo[deviceIndex]->setDevicePowerStat(stat);
        }
    }
    return false;
}

bool IntelAudioRpmDevice::setAllDevicePowerStat(const QString &stat)
{
    if (stat == "0" || stat == "1") {
        for (int i = 0; i < m_devicePowerInfo.count(); ++i) {
            m_devicePowerInfo[i]->setDevicePowerStat(stat);
        }
        return true;
    }
    return false;
}
