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

#include "devpowercontrol.h"
#include <QDebug>

DevPowerControl::DevPowerControl(QObject *parent) : QObject(parent)
{
    InitRpmDeviceMap();
}

DevPowerControl::~DevPowerControl() {}

void DevPowerControl::setAllDevicePerformance()
{
    setAllDevice("on");
}

void DevPowerControl::setAllDeviceBalance()
{
    setAllDevice("default");
}

void DevPowerControl::setAllDeviceSave()
{
    setAllDevice("auto");
}

//不在调用 转到SetDevicePowerCtl这个接口
void DevPowerControl::setAllDevice(QString mode)
{
    // if (m_pci != nullptr) {
    //     m_pci->setAllDevicePowerStat(mode);
    // }

    // if (m_i2c != nullptr) {
    //     m_i2c->setAllDevicePowerStat(mode);
    // }

    // if (m_sata != nullptr) {
    //     if (mode == "on") {
    //         m_sata->setAllDevicePowerStat("max_performance");
    //     } else if (mode == "auto") {
    //         m_sata->setAllDevicePowerStat("min_power");
    //     } else {
    //         m_sata->setAllDevicePowerStat(mode);
    //     }
    // }
}

void DevPowerControl::SetDevicePowerCtl(int mode, QString scene)
{
        m_powerPolicy = mode;
        QString value = nullptr;

        for(const auto &devRpmItem : m_rpmDeviceMap) {
            value = UpmCustomHardwareConfig::self()->getScenekeyValue(mode, scene, devRpmItem.first, devRpmItem.second->getPowerConfig());
            qDebug() << devRpmItem.first << ", mode:"<< mode << "scene:" << scene << ", configValue: " << value;
            devRpmItem.second->setAllDevicePowerStat(value);
        }
}

void DevPowerControl::InitRpmDeviceMap()
{
    QSettings *m_globalConfig = new QSettings(UPM_GLOBAL_HARDWARE_CONFIG_FILE, QSettings::IniFormat, this);
    m_globalConfig->beginGroup(UPM_GROUP_HARDWARE_CONTROL);
    if (m_globalConfig->value(UPM_GROUP_DEVFREQPOLICY).toInt() == 1) {
        m_rpmDeviceMap[UPM_GROUP_DEVFREQPOLICY] = std::unique_ptr<DevRpmInterface>(new DevFreqPolicyRpm(this));
    }

    if (m_globalConfig->value(UPM_GROUP_USBRPM).toInt() == 1) {
        m_rpmDeviceMap[UPM_GROUP_USBRPM] = std::unique_ptr<DevRpmInterface>(new UsbRpmDevice(this));
    }

    if (m_globalConfig->value(UPM_GROUP_PCIRPM).toInt() == 1) {
        m_rpmDeviceMap[UPM_GROUP_PCIRPM] = std::unique_ptr<DevRpmInterface>(new PciRpmDevice(this));
    }

    if (m_globalConfig->value(UPM_GROUP_I2CRPM).toInt() == 1) {
        m_rpmDeviceMap[UPM_GROUP_I2CRPM] = std::unique_ptr<DevRpmInterface>(new I2cRpmDevice(this));
    }

    if (m_globalConfig->value(UPM_GROUP_PCIEASPM).toInt() == 1) {
        m_rpmDeviceMap[UPM_GROUP_PCIEASPM] = std::unique_ptr<DevRpmInterface>(new PcieAspmDevice(this));
    }

    if (m_globalConfig->value(UPM_GROUP_SATALINKPOWER).toInt() == 1) {
        m_rpmDeviceMap[UPM_GROUP_SATALINKPOWER] = std::unique_ptr<DevRpmInterface>(new SataLinkRpmDevice(this));
    }
    if (m_globalConfig->value(UPM_GROUP_SNAHDAINTELAUDIO).toInt() == 1) {
        m_rpmDeviceMap[UPM_GROUP_SNAHDAINTELAUDIO] = std::unique_ptr<DevRpmInterface>(new IntelAudioRpmDevice(this));
    }

    if (m_globalConfig->value(UPM_GROUP_NETWOl).toInt() == 1) {
        m_rpmDeviceMap[UPM_GROUP_NETWOl] = std::unique_ptr<DevRpmInterface>(new NetWolRpmDevice(this));
    }
    if (m_globalConfig->value(UPM_GROUP_WAKEUPSOURCE).toInt() == 1) {
        m_rpmDeviceMap[UPM_GROUP_WAKEUPSOURCE] = std::unique_ptr<DevRpmInterface>(new WakeSourceRpmDevice(this));
    }

    if (m_globalConfig->value(UPM_GROUP_KEYBOARDBACKLIGHT).toInt() == 1) {
        m_rpmDeviceMap[UPM_GROUP_KEYBOARDBACKLIGHT] = std::unique_ptr<DevRpmInterface>(new KeyboardBacklightDevice(this));
    }

    if (m_globalConfig->value(UPM_GROUP_TOUCHSCREEN).toInt() == 1) {
        m_rpmDeviceMap[UPM_GROUP_TOUCHSCREEN] = std::unique_ptr<DevRpmInterface>(new TouchScreenDevice(this));
    }
    m_globalConfig->endGroup();
}

