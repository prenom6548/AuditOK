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
#include <QDirIterator>
#include "wakesourcerpm.h"

WakeSourceRpmDevice::WakeSourceRpmDevice(QObject *parent) : QObject(parent)
{
    m_powerConfigMap = UpmCustomHardwareConfig::self()->getConfig(UPM_GROUP_WAKEUPSOURCE);
    printConfigMap();
    getDevicePowerInfo();
}

void WakeSourceRpmDevice::getDevicePowerInfo()
{
    QDirIterator it(WAKE_SOURCE_PATH, QStringList() << "wakeup", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        const QString wakeupFile = it.next();

        QString currentValue;
        if (!readFirstLine(wakeupFile, currentValue)) {
            continue;
        }

        const QString devPath = extractDevicePathFromWakeupFile(wakeupFile);
        const QString devName = extractDeviceNameFromPath(devPath);

        if (!(isUsbDevice(devPath))) {
           continue;
        }

        m_devicePowerInfo.push_back(new DevicePowerInfo("wakeup", devPath, devName, "/power/wakeup", this));
        qInfo() << "wakeupdevice: add" << devName << "at" << devPath << "value" << currentValue;
    }
}

bool WakeSourceRpmDevice::setPowerStat(int deviceIndex, const QString &stat)
{

    if (stat == "enabled" || stat == "disabled") {
        if (deviceIndex < m_devicePowerInfo.count()) {
            return m_devicePowerInfo[deviceIndex]->setDevicePowerStat(stat);
        }
    }
    return false;
}

bool WakeSourceRpmDevice::setAllDevicePowerStat(const QString &stat)
{
    qDebug() << "WakeSourceRpmDevice::setAllDevicePowerStat: " << stat;
    if (stat == "enabled" || stat == "disabled") {
        for (int i = 0; i < m_devicePowerInfo.count(); ++i) {
            m_devicePowerInfo[i]->setDevicePowerStat(stat);
        }
        return true;
    }
    return false;
}

QString WakeSourceRpmDevice::extractDevicePathFromWakeupFile(const QString &wakeupFilePath)
{
    QFileInfo fi(wakeupFilePath);
    const QString powerDir = fi.dir().absolutePath();
    const QString devPath = QFileInfo(powerDir + "/..").canonicalFilePath();
    return devPath.isEmpty() ? QFileInfo(powerDir + "/..").absoluteFilePath() : devPath;
}

bool WakeSourceRpmDevice::isUsbDevice(const QString &devicePath)
{
    return devicePath.contains("/usb", Qt::CaseInsensitive);
}

bool WakeSourceRpmDevice::readFirstLine(const QString &filePath, QString &out)
{
    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    QTextStream ts(&f);
    out = ts.readLine();
    f.close();
    return true;
}
