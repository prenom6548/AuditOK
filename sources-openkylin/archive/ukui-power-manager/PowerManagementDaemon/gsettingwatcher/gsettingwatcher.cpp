/*
 * Copyright 2021 KylinSoft Co., Ltd.
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

#include "gsettingwatcher.h"

GsettingWatcher::GsettingWatcher() {}

GsettingWatcher::~GsettingWatcher() {}

void GsettingWatcher::initGsettingWatcher()
{
    mAcWatcher.initAcWatcher();

    initSettingsConnect();

    initLowPowerAutoSave();
}

void GsettingWatcher::initSettingsConnect()
{
    mPowerPolicyAc = m_powerManagerGsettings->get(POWER_POLICY_AC).toInt();
    mPowerPolicyBattery = m_powerManagerGsettings->get(POWER_POLICY_BATTERY).toInt();
    if (false == mAcWatcher.mPowerState) {
        policyAC();
    } else {
        policyBattery();
    }

    connect(m_powerManagerGsettings, &QGSettings::changed, this, [=](const QString &key) {
        if (key == POWER_POLICY_AC) {
            mPowerPolicyAc = m_powerManagerGsettings->get(POWER_POLICY_AC).toInt();
            if (false == mAcWatcher.mPowerState) {
                policyAC();
            }
        }
        if (key == POWER_POLICY_BATTERY) {
            mPowerPolicyBattery = m_powerManagerGsettings->get(POWER_POLICY_BATTERY).toInt();
            if (true == mAcWatcher.mPowerState) {
                policyBattery();
                if (EnergySaving != mPowerPolicyBattery && Unknow != m_powerPolicyBatteryBackup) {
                    m_powerPolicyBatteryBackup = Unknow;
                    m_powerManagerGsettings->set(POWER_POLICY_BATTERY_BACKUP, Unknow);
                }
            }
        }
    });

    connect(&mAcWatcher, SIGNAL(acChanged(bool)), this, SLOT(AcWatcherChanged(bool)));
}

void GsettingWatcher::AcWatcherChanged(const bool &onBattery)
{
    if (false == onBattery) {
        policyAC();
        setLowPowerState(false);
    } else {
        if (m_percentageLow >= m_batteryPercentage) {
            setLowPowerState(true);
        } else {
            policyBattery();
        }
    }
    // 直接传入已知的电源状态，避免重复调用readOnBattery()
    handlePowerStateChange(onBattery);
}

void GsettingWatcher::setLowPowerState(bool lowPowerState)
{
    if (mLowPrecentState == lowPowerState) {
        return;
    }
    if (true == lowPowerState) {
        if (1 == mLowBatteryAutoSave) {
            if (Unknow == m_powerPolicyBatteryBackup) {
                m_powerPolicyBatteryBackup = mPowerPolicyBattery;
                m_powerManagerGsettings->set(POWER_POLICY_BATTERY, EnergySaving);
                m_powerManagerGsettings->set(POWER_POLICY_BATTERY_BACKUP, m_powerPolicyBatteryBackup);
                qInfo() << "battery policy backup:" << m_powerPolicyBatteryBackup;
            }
        }
    } else {
        resetPowerPolicyBatteryBackup();
        qDebug() << "exit low battery mode";
    }

    mLowPrecentState = lowPowerState;
}

void GsettingWatcher::initLowPowerAutoSave()
{
    mLowBatteryAutoSave = m_powerManagerGsettings->get(LOW_BATTERY_ATUO_SAVE).toInt();
    m_powerPolicyBatteryBackup = m_powerManagerGsettings->get(POWER_POLICY_BATTERY_BACKUP).toInt();
    m_percentageLow = m_powerManagerGsettings->get(PERCENTAGE_LOW).toInt();

    QDBusInterface dBusInterface(UPOWER_SERVICE,
                                 UPOWER_DISPLAY_PATH,
                                 FREEDESKTOP_UPOWER,
                                 QDBusConnection::systemBus());
    QDBusReply<QVariant> reply = dBusInterface.call("Get", "org.freedesktop.UPower.Device", "Percentage");
    if (reply.isValid()) {
        m_batteryPercentage = reply.value().toDouble();
    } else {
        qDebug() << "get power percentage failed!";
        m_batteryPercentage = 0;
    }

    if (true == mAcWatcher.mPowerState) {
        if (m_percentageLow >= m_batteryPercentage) {
            setLowPowerState(true);
        } else {
            resetPowerPolicyBatteryBackup();
        }
    } else {
        resetPowerPolicyBatteryBackup();
    }

    connect(m_powerManagerGsettings, &QGSettings::changed, this, [=](const QString &key) {
        if (key == LOW_BATTERY_ATUO_SAVE) {
            qInfo() << "low battery auto save changed" << mLowBatteryAutoSave;
            int newValue = m_powerManagerGsettings->get(LOW_BATTERY_ATUO_SAVE).toInt();
            if (mLowBatteryAutoSave == newValue) {
                return;
            }
            mLowBatteryAutoSave = m_powerManagerGsettings->get(LOW_BATTERY_ATUO_SAVE).toInt();
            if (1 == mLowBatteryAutoSave) {
                if (true == mLowPrecentState) {
                    m_powerPolicyBatteryBackup = mPowerPolicyBattery;
                    m_powerManagerGsettings->set(POWER_POLICY_BATTERY, EnergySaving);
                }
            } else {
                if (true == mLowPrecentState) {
                    m_powerPolicyBatteryBackup = Unknow;
                }
            }
            m_powerManagerGsettings->set(POWER_POLICY_BATTERY_BACKUP, m_powerPolicyBatteryBackup);
        }
        if (key == PERCENTAGE_LOW) {
            qInfo() << "percentage low changed" << m_percentageLow;
            int newPercentageLowValue = m_powerManagerGsettings->get(PERCENTAGE_LOW).toInt();
            if (m_percentageLow == newPercentageLowValue) {
                return;
            }
            m_percentageLow = m_powerManagerGsettings->get(PERCENTAGE_LOW).toInt();
            if (true == mAcWatcher.mPowerState) {
                if (m_percentageLow >= m_batteryPercentage) {
                    setLowPowerState(true);
                } else {
                    setLowPowerState(false);
                }
            }
        }
    });

    QDBusConnection::systemBus().connect(
                UPOWER_SERVICE, UPOWER_DISPLAY_PATH, FREEDESKTOP_UPOWER,
                "PropertiesChanged", this, SLOT(dealBatteryPropertiesMessage(QDBusMessage)));
}

void GsettingWatcher::resetPowerPolicyBatteryBackup()
{
    m_powerPolicyBatteryBackup = m_powerManagerGsettings->get(POWER_POLICY_BATTERY_BACKUP).toInt();
    if (Unknow != m_powerPolicyBatteryBackup) {
        m_powerManagerGsettings->set(POWER_POLICY_BATTERY, m_powerPolicyBatteryBackup);
        m_powerPolicyBatteryBackup = Unknow;
        m_powerManagerGsettings->set(POWER_POLICY_BATTERY_BACKUP, m_powerPolicyBatteryBackup);
    }
}

void GsettingWatcher::dealBatteryPropertiesMessage(QDBusMessage msg)
{
    const QDBusArgument &dbusArgs = msg.arguments().at(1).value<QDBusArgument>();
    QMap<QString, QVariant> map;
    dbusArgs >> map;

    if (map.contains("Percentage")) {
        int batteryPercentage = map.value(QString("Percentage")).toDouble();
        if (m_batteryPercentage != batteryPercentage) {
            m_batteryPercentage = batteryPercentage;
            if (true == mAcWatcher.mPowerState) {
                if (m_percentageLow >= m_batteryPercentage) {
                    setLowPowerState(true);
                }
            }
        }
    }
}

void GsettingWatcher::policyAC()
{
    controlPowerManagement(CPU_FREQUENCY_MODULATION, mPowerPolicyAc);
    controlPowerManagement(GPU_FREQUENCY_MODULATION, mPowerPolicyAc);
    // controlPowerManagement(SET_AUDIO_MODE, mPowerPolicyAc);
    // controlPowerManagement(SET_PCIE_ASPM_MODE, mPowerPolicyAc);
    controlPowerManagement(SET_DEVICE_POWER_CTL, mPowerPolicyAc);
    m_powerManagerGsettings->set(POWER_POLICY_CURRENT, mPowerPolicyAc);
}

void GsettingWatcher::policyBattery()
{
    controlPowerManagement(CPU_FREQUENCY_MODULATION, mPowerPolicyBattery);
    controlPowerManagement(GPU_FREQUENCY_MODULATION, mPowerPolicyBattery);
    // controlPowerManagement(SET_AUDIO_MODE, mPowerPolicyBattery);
    // controlPowerManagement(SET_PCIE_ASPM_MODE, mPowerPolicyBattery);
    controlPowerManagement(SET_DEVICE_POWER_CTL, mPowerPolicyBattery);
    m_powerManagerGsettings->set(POWER_POLICY_CURRENT, mPowerPolicyBattery);
}
