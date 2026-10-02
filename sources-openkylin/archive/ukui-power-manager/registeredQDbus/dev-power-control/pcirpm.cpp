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
#include <QString>
#include <QStringList>
#include <QDebug>
#include <iostream>

#include "pcirpm.h"

static struct pci_access *g_pciAccess = NULL;

PciRpmDevice::PciRpmDevice(QObject *parent) : QObject(parent)
{
    m_powerConfigMap = UpmCustomHardwareConfig::self()->getConfig(UPM_GROUP_PCIRPM);
    m_devFilterRulesMap = UpmCustomHardwareConfig::self()->getConfig(FILTER_PCI_CLASS);
    printConfigMap();
    m_controlFile = QString("/power/control");

    getDevicePowerInfo();
    getBlockDevicePowerInfo();
}

void PciRpmDevice::getAtaDevicePowerInfo(const QString &parentDir)
{
    QDir ataDeviceDir(parentDir);
    if(!ataDeviceDir.exists()) {
        return;
    }

    QStringList ataDeviceDirs = ataDeviceDir.entryList(QDir::Dirs).filter("ata");

    for (int i = 0; i < ataDeviceDirs.size(); ++i) {
        QString devicePath = parentDir + '/' + ataDeviceDirs.at(i);

        if (true == deviceHasRuntimePM(devicePath)) {
            QString devName = lookupPciDeviceName(devicePath);
            m_devicePowerInfo.push_back(new DevicePowerInfo("pci-ata", devicePath, devName, m_controlFile, this));
        }
    }
}

void PciRpmDevice::getDevicePowerInfo()
{
    QDir deviceDir(PCI_DEVICE_PATH);
    if(!deviceDir.exists()) {
        return;
    }

    QStringList deviceDirs = deviceDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (int i = 0; i < deviceDirs.size(); ++i) {
        QString devicePath = PCI_DEVICE_PATH + deviceDirs.at(i);

        if (true == deviceHasRuntimePM(devicePath)) {
            QString devName = lookupPciDeviceName(devicePath);
            m_devicePowerInfo.push_back(new DevicePowerInfo("pci", devicePath, devName, m_controlFile, this));
        }

        getAtaDevicePowerInfo(devicePath);
    }
}

void PciRpmDevice::getBlockDevicePowerInfo()
{
    QDir blockDeviceDir(PCI_BLOCK_DEVICE_PATH);
    if(!blockDeviceDir.exists()) {
        return;
    }

    QStringList blockDeviceDirs = blockDeviceDir.entryList(QDir::Dirs).filter("sd");
    for (int i = 0; i < blockDeviceDirs.size(); ++i) {
        QString controlFilePath = PCI_BLOCK_DEVICE_PATH + blockDeviceDirs.at(i) + "/device";

        if (true == deviceHasRuntimePM(controlFilePath)) {
            m_devicePowerInfo.push_back(new DevicePowerInfo("pci-block", controlFilePath, blockDeviceDirs.at(i), m_controlFile, this));
        }
    }
}

bool PciRpmDevice::deviceHasRuntimePM(const QString &devPath)
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

QString PciRpmDevice::lookupPciDeviceName(const QString &devPath)
{
    ulong vendorId = 0;
    ulong deviceId = 0;
    bool ok;
    QString devNameStr;

    QFile file(devPath + "/vendor");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        vendorId = file.readLine().toULong(&ok, 16);
        file.close();
    }

    file.setFileName(devPath + "/device");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        deviceId = file.readLine().toULong(&ok, 16);
        file.close();
    }

    char devName[512] = {0};
    char *pDevName = NULL;

    if (NULL == g_pciAccess) {
        g_pciAccess = pci_alloc();
        pci_init(g_pciAccess);
    }

    pDevName = pci_lookup_name(g_pciAccess, devName, 512, PCI_LOOKUP_VENDOR | PCI_LOOKUP_DEVICE, vendorId, deviceId);

    QString pciClassStr;
    QString pciClass;
    QString shortClassHex;
    QFile classfile(devPath + "/class");
    if (classfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        pciClassStr = classfile.readLine().trimmed();
        classfile.close();
    }

    if (!pciClassStr.isEmpty()) {
        uint32_t fullClass = pciClassStr.toUInt(nullptr, 16);
        uint16_t shortClass = static_cast<uint16_t>(fullClass >> 8);
        shortClassHex = QString("0x%1").arg(shortClass, 4, 16, QChar('0'));
        pciClass = getPciClassName(shortClassHex.toUInt(nullptr, 16));
    }

    if (!shortClassHex.isEmpty()) {
        devNameStr = QString("%1 [%2]: %3")
                        .arg(pciClass).arg(shortClassHex).arg(pDevName);
    } else {
        devNameStr = QString(pDevName);
    }

    return devNameStr;
}

bool PciRpmDevice::setPowerStat(int deviceIndex, const QString &stat)
{
    if (stat == "auto" || stat == "on" || stat == "default") {
        if (deviceIndex < m_devicePowerInfo.count()) {
            return m_devicePowerInfo[deviceIndex]->setDevicePowerStat(stat);
        }
    }
    return false;
}

bool PciRpmDevice::setAllDevicePowerStat(const QString &stat)
{
    qDebug() << "PciRpmDevice::setAllDevicePowerStat: " << stat;
    if (stat == "auto" || stat == "on" || stat == "default") {
        for (int i = 0; i < m_devicePowerInfo.count(); ++i) {
            if(ignoreDeviceList(m_devicePowerInfo[i]->getDeviceName())) continue;
            m_devicePowerInfo[i]->setDevicePowerStat(stat);
        }
        return true;
    }
    return false;
}

bool PciRpmDevice::ignoreDeviceList(QString deviceName)
{
    //使用PCI类型替代字符串匹配
    for (auto it = m_devFilterRulesMap.begin(); it != m_devFilterRulesMap.end(); ++it) {
        const QString &ruleName = it.key();
        const QString &rulePattern = it.value();

        if (rulePattern.isEmpty()) {
            continue;
        }

        if (deviceName.contains(rulePattern, Qt::CaseInsensitive)) {
            qDebug() << QString("ignore set pci %1").arg(deviceName) << ruleName;
            return true;
        }
    }

    return false;
}
