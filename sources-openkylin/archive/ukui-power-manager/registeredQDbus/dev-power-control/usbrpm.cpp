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

#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QProcess>
#include <QDebug>
#include <QThread>
#include "usbrpm.h"

#include <iostream>

UsbRpmDevice::UsbRpmDevice(QObject *parent) : QObject(parent)
{
    m_powerConfigMap = UpmCustomHardwareConfig::self()->getConfig(UPM_GROUP_USBRPM);
    m_devFilterRulesMap = UpmCustomHardwareConfig::self()->getConfig(FILTER_USB_CLASS);
    m_devIgnoreListMap = UpmCustomHardwareConfig::self()->getConfig(IGNORE_USB_DEV);
    printConfigMap();
    m_controlFile = QString("/power/control");
    getDevicePowerInfo();
}

void UsbRpmDevice::getDevicePowerInfo()
{
    QDir deviceDir(USB_DEVICE_PATH);
    if (!deviceDir.exists()) {
        return;
    }

    QStringList deviceDirs = deviceDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (int i = 0; i < deviceDirs.size(); ++i) {
        QString devPath = USB_DEVICE_PATH + deviceDirs.at(i);
        if (true == deviceHasRuntimePM(devPath) && !checkSubdevicesForIgnore(devPath)) {
            QString devName = getUsbDeviceName(devPath);
            qDebug() << "Usbdevice: adding device with Runtime PM:" << devName << "at" << devPath;
            m_devicePowerInfo.push_back(new DevicePowerInfo("usb", devPath, devName, m_controlFile, this));
        }
    }
}

bool UsbRpmDevice::deviceHasRuntimePM(const QString &devPath)
{
    QFile control(devPath + m_controlFile);
    if (!control.exists()) {
        return false;
    }

    QFile pmFile(devPath + "/power/runtime_suspended_time");
    if (pmFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        ulong v = pmFile.readLine().toULong();
        pmFile.close();
        if (v) return true;
    }

    pmFile.setFileName(devPath + "/power/runtime_active_time");
    if (pmFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        ulong v = pmFile.readLine().toULong();
        pmFile.close();
        if (v) return true;
    }

    return false;
}

QString UsbRpmDevice::getUsbDeviceName(const QString &devPath)
{
    QFile f(devPath + "/product");
    if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString name = f.readLine();
        f.close();
        return name.remove('\n');
    }
    // 兜底用目录名
    return QFileInfo(devPath).fileName();
}

bool UsbRpmDevice::setPowerStat(int deviceIndex, const QString &stat)
{
    // 非 SATA 设备：支持 "auto" / "on" / "default"（由 DevicePowerInfo 处理）
    if (stat == "auto" || stat == "on" || stat == "default") {
        if (deviceIndex < m_devicePowerInfo.count()) {
            return m_devicePowerInfo[deviceIndex]->setDevicePowerStat(stat);
        }
    }
    return false;
}

bool UsbRpmDevice::setAllDevicePowerStat(const QString &stat)
{
    qDebug() << "usbdevice: Setting all device to power stat:" << stat;
    if (stat == "auto" || stat == "on" || stat == "default") {
        for (int i = 0; i < m_devicePowerInfo.count(); ++i) {
            m_devicePowerInfo[i]->setDevicePowerStat(stat);
        }
        return true;
    }
    return false;
}

bool UsbRpmDevice::matchInterfaceRule(const QString &ifClass, const QString &ifName,
                                       const QString &ifSubclass, const QString &ifProto,
                                       const QString &rulePattern)
{
    const int RULE_PARTS_COUNT = 4;
    QStringList ruleParts = rulePattern.split(":");

    if (ruleParts.size() < RULE_PARTS_COUNT) {
        return false;
    }

    QString ruleClass = ruleParts[0].trimmed();
    QString ruleName = ruleParts[1].trimmed();
    QString ruleSubclass = ruleParts[2].trimmed();
    QString ruleProto = ruleParts[3].trimmed();

    bool classMatch = ruleClass.isEmpty() || ifClass.contains(ruleClass, Qt::CaseInsensitive);
    bool nameMatch = ruleName.isEmpty() || ifName.contains(ruleName, Qt::CaseInsensitive);
    bool subclassMatch = ruleSubclass.isEmpty() || ifSubclass.contains(ruleSubclass, Qt::CaseInsensitive);
    bool protoMatch = ruleProto.isEmpty() || ifProto.contains(ruleProto, Qt::CaseInsensitive);

    return classMatch && nameMatch && subclassMatch && protoMatch;
}

bool UsbRpmDevice::checkSubdevicesForIgnore(const QString &devicePath)
{
    QDir mainDir(devicePath);
    if (!mainDir.exists()) {
        qDebug() << "usb device path not exist:" << devicePath;
        return true;
    }

    // ========== Step 1: Read main device VID/PID (only once) ==========
    QString devVid, devPid;
    if (!UpmCommonFile::read(devicePath, "/idVendor", devVid)) {
        qDebug() << "usb read VID fail:" << devicePath;
    }
    if (!UpmCommonFile::read(devicePath, "/idProduct", devPid)) {
        qDebug() << "usb read PID fail:" << devicePath;
    }
    devVid = devVid.trimmed().toLower();
    devPid = devPid.trimmed().toLower();
    //qDebug() << "usb device VID:PID =" << devVid << ":" << devPid;

    // ========== Step 2: Check VID/PID against ignore list ==========
    for (auto it = m_devIgnoreListMap.begin(); it != m_devIgnoreListMap.end(); ++it) {
        const QString &ignoreName = it.key();
        const QString &vidPidPattern = it.value();

        QStringList vidPidParts = vidPidPattern.split(":");
        if (vidPidParts.size() >= 2) {
            QString ruleVid = vidPidParts[0].trimmed().toLower();
            QString rulePid = vidPidParts[1].trimmed().toLower();

            bool vidMatch = ruleVid.isEmpty() || devVid.contains(ruleVid, Qt::CaseInsensitive);
            bool pidMatch = rulePid.isEmpty() || devPid.contains(rulePid, Qt::CaseInsensitive);

            if (vidMatch && pidMatch) {
                qInfo() << "usb ignore:" << ignoreName << "(" << vidPidPattern << ")" << devicePath;
                return true;
            }
        }
    }

    // ========== Step 3: Get sub-device list ==========
    QStringList filters;
    filters << "*:*";  // Sub-device name pattern (contains ":")
    mainDir.setNameFilters(filters);
    mainDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    QStringList subDevNames = mainDir.entryList();

    if (subDevNames.isEmpty()) {
        qDebug() << "usb sub-device not found:" << devicePath;
        return false;  // No sub-devices to check
    }

    // ========== Step 4: Check interface class rules ==========
    for (const QString &subDevName : subDevNames) {
        QString subDevPath = mainDir.filePath(subDevName);
        qDebug() << "usb checking sub-device:" << subDevPath;

        // Read interface information
        QString ifClass, ifName, ifSubclass, ifProto;
        UpmCommonFile::read(subDevPath, "/bInterfaceClass", ifClass);
        UpmCommonFile::read(subDevPath, "/interface", ifName);
        UpmCommonFile::read(subDevPath, "/bInterfaceSubClass", ifSubclass);
        UpmCommonFile::read(subDevPath, "/bInterfaceProtocol", ifProto);

        QString ifClassFmt = ifClass.trimmed().rightJustified(2, '0', true);
        QString className = USB_CLASS_MAP.value(ifClassFmt, "Unknown Class");

        qDebug() << "usb interface info - class:" << ifClass << "(" << className << ")"
                 << "name:" << ifName << "subclass:" << ifSubclass << "protocol:" << ifProto;

        // Match against filter rules
        for (auto it = m_devFilterRulesMap.begin(); it != m_devFilterRulesMap.end(); ++it) {
            const QString &ruleName = it.key();
            const QString &rulePattern = it.value();

            if (!matchInterfaceRule(ifClass, ifName, ifSubclass, ifProto, rulePattern)) {
                continue;
            }

            qInfo() << "usb filter rule matched:" << ruleName << "(" << rulePattern << ")" << devicePath << ifName;
            return true;  // Ignore this device
        }
    }

    return false;  // Device passed all checks, should not be ignored
}