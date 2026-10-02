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

#ifndef USBDEVICE_H
#define USBDEVICE_H

#include "devrpminterface.h"
#include "../upmcommonfile.h"

#define USB_DEVICE_PATH "/sys/bus/usb/devices/"

static const QMap<QString, QString> USB_CLASS_MAP = {
    {"00", "Unspecified (Vendor Specific)"},
    {"01", "Audio"},          // 音频设备
    {"02", "Communications"}, // 通信设备
    {"03", "HID"},            // 人机交互设备
    {"05", "Physical"},       // 物理设备
    {"06", "Image"},          // 图像设备
    {"07", "Printer"},        // 打印机
    {"08", "Mass Storage "},  //存储设备
    {"09", "Hub"},            //USB集线器
    {"0a", "CDC-Data"},       //WWAN数据设备
    {"0b", "Smart Card"},     //智能卡设备
    {"0c", "Content Security"},       //安全设备
    {"0e", "Video"},          //视频设备
    {"0f", "Personal Healthcare"},    //医疗设备
    {"10", "Audio/Video"},    //音视频复合设备
    {"11", "Billboard"},      //Billboard设备
    {"dc", "Diagnostic Device"},     //诊断设备
    {"e0", "Wireless Controller"},   //无线控制器
    {"ef", "Miscellaneous"},         //多功能设备
    {"fe", "Application Specific"},  //应用专用设备
    {"ff", "Vendor Specific"}        //厂商自定义设备
};

class UsbRpmDevice : public QObject, public DevRpmInterface
{
    Q_OBJECT
public:
    explicit UsbRpmDevice(QObject *parent = nullptr);

private:
    QVector<DevicePowerInfo *> m_devicePowerInfo;
    QString m_controlFile;

    void getDevicePowerInfo();
    bool deviceHasRuntimePM(const QString &devPath);
    QString getUsbDeviceName(const QString &devPath);
    bool checkSubdevicesForIgnore(const QString &devicePath);
    bool matchInterfaceRule(const QString &ifClass, const QString &ifName,
                                       const QString &ifSubclass, const QString &ifProto,
                                       const QString &rulePattern);

public:
    bool setPowerStat(int deviceIndex, const QString &stat);
    bool setAllDevicePowerStat(const QString &stat);
};

#endif // USBDEVICE_H
