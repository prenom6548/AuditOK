/*
 * Copyright 2024 KylinSoft Co., Ltd.
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

#include "upmcustomdefaultgsettings.h"
#include "eventwatcher.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>

#include "common.h"

enum policy
{
    Performance = 0,
    Balance,
    EnergySaving,
};

UpmCustomDefaultGsettings::UpmCustomDefaultGsettings(QObject *parent) : QObject(parent)
{
    m_upmGsettings = new QGSettings(QByteArray(POWER_MANAGER_SETTINGS), QByteArray(), this);
    if (true == m_upmGsettings->get(PRE_POWER_MODE).toBool()) {
        return;
    }

    getHardwareInfo();

    m_globalConfig = new QSettings(UPM_GLOBAL_GSETTINGS_CONFIG_FILE, QSettings::IniFormat, this);

    QString extendConfigFilePath;
    if (true == getExtendConfigFile(extendConfigFilePath)) {
        m_extendConfig = new QSettings(extendConfigFilePath, QSettings::IniFormat, this);
        m_extendConfig->beginGroup("product");
        QStringList productList = m_extendConfig->allKeys();
        Q_FOREACH (const QString &product, productList) {
            if (m_hardwareInfo.contains(product, Qt::CaseInsensitive)) {
                if (1 == m_extendConfig->value(product).toInt()) {
                    m_productGroup = product;
                }
                break;
            }
        }
        m_extendConfig->endGroup();
    }
}

UpmCustomDefaultGsettings::~UpmCustomDefaultGsettings() {}

void UpmCustomDefaultGsettings::setUpmCustomDefaultGsettings()
{
    if (true == m_upmGsettings->get(PRE_POWER_MODE).toBool()) {
        return;
    }

    setCustomDefaultCpuPolicyAc();

    setCustomDefaultCpuPolicyBattery();

    setCustomDefaultIdleBrightness();

    setCustomDefaultIdlePowerPolicyAuto();

    setCustomDefaultSleepComputerAc();

    setCustomDefaultSleepComputerBattery();

    setCustomDefaultIdleCloseDisplayTime();

    m_upmGsettings->set(PRE_POWER_MODE, true);
}

void UpmCustomDefaultGsettings::getHardwareInfo()
{
    if (false == readAll(UPM_HARDWARE_INFO_FILE, m_hardwareInfo)) {
        qDebug() << "get hardware info failed";
    }
}

void UpmCustomDefaultGsettings::setCustomDefaultCpuPolicyAc()
{
    if (true == getGlobalCpuDefaultPolicy(UPM_GROUP_DEFAULT_POLICY_AC,
                                          UPM_KEY_BALANCE_MODE_BOARD_NAME)) {
        m_upmGsettings->set(POWER_POLICY_AC, Balance);
        return ;
    }
    if (true == getGlobalCpuDefaultPolicy(UPM_GROUP_DEFAULT_POLICY_AC,
                                          UPM_KEY_POWERSAVE_MODE_BOARD_NAME)) {
        m_upmGsettings->set(POWER_POLICY_AC, EnergySaving);
        return ;
    }

    if (nullptr != m_extendConfig) {
        QVariant value;
        if (true == readConfig(m_extendConfig,
                               UPM_GROUP_EXTEND_BRAND_GENERIC_CONFIG,
                               POWER_POLICY_AC, value)) {
            m_upmGsettings->set(POWER_POLICY_AC, value.toInt());
        }
        if (true == readConfig(m_extendConfig, m_productGroup, POWER_POLICY_AC, value)) {
            m_upmGsettings->set(POWER_POLICY_AC, value.toInt());
        }
    }
}

void UpmCustomDefaultGsettings::setCustomDefaultCpuPolicyBattery()
{
    if (true == getGlobalCpuDefaultPolicy(UPM_GROUP_DEFAULT_POLICY_BATTERY,
                                          UPM_KEY_BALANCE_MODE_BOARD_NAME)) {
        m_upmGsettings->set(POWER_POLICY_BATTERY, Balance);
        return ;
    }

    if (true == getGlobalCpuDefaultPolicy(UPM_GROUP_DEFAULT_POLICY_BATTERY,
                                          UPM_KEY_POWERSAVE_MODE_BOARD_NAME)) {
        m_upmGsettings->set(POWER_POLICY_BATTERY, EnergySaving);
        return ;
    }

    if (nullptr != m_extendConfig) {
        QVariant value;
        if (true == readConfig(m_extendConfig,
                               UPM_GROUP_EXTEND_BRAND_GENERIC_CONFIG,
                               POWER_POLICY_BATTERY, value)) {
            m_upmGsettings->set(POWER_POLICY_BATTERY, value.toInt());
        }
        if (true == readConfig(m_extendConfig, m_productGroup, POWER_POLICY_BATTERY, value)) {
            m_upmGsettings->set(POWER_POLICY_BATTERY, value.toInt());
        }
    }
}

void UpmCustomDefaultGsettings::setCustomDefaultIdleBrightness()
{
    if (nullptr != m_extendConfig) {
        QVariant value;
        if (true == readConfig(m_extendConfig, m_productGroup, IDLE_BRIGHTNESS, value)) {
            m_upmGsettings->set(IDLE_BRIGHTNESS, value.toInt());
        }
    }
}

void UpmCustomDefaultGsettings::setCustomDefaultIdlePowerPolicyAuto()
{
    if (nullptr != m_extendConfig) {
        QVariant value;
        if (true == readConfig(m_extendConfig, m_productGroup, IDLE_POWER_POLICY_AUTO, value)) {
            if (1 == value.toInt()) {
                m_upmGsettings->set(IDLE_POWER_POLICY_AUTO, true);
            }
        }
    }
}

void UpmCustomDefaultGsettings::setCustomDefaultSleepComputerAc()
{
    if (nullptr != m_extendConfig) {
        QVariant value;
        if (true == readConfig(m_extendConfig, m_productGroup, SLEEP_COMPUTER_AC, value)) {
            m_upmGsettings->set(SLEEP_COMPUTER_AC, value.toInt());
        }
    }
}

void UpmCustomDefaultGsettings::setCustomDefaultSleepComputerBattery()
{
    if (nullptr != m_extendConfig) {
        QVariant value;
        if (true == readConfig(m_extendConfig, m_productGroup, SLEEP_COMPUTER_BATTERY, value)) {
            m_upmGsettings->set(SLEEP_COMPUTER_BATTERY, value.toInt());
        }
    }
}

void UpmCustomDefaultGsettings::setCustomDefaultIdleCloseDisplayTime()
{
    bool isFlemingX = EventWatcher::checkDeviceModel("isFlemingXModel");
    if(isFlemingX) {
        m_upmGsettings->set(SLEEP_DISPLAY_AC_KEY, 300);
        m_upmGsettings->set(SLEEP_DISPLAY_BAT_KEY, 180);
    }
}

bool UpmCustomDefaultGsettings::getGlobalCpuDefaultPolicy(const QString &group, const QString &key)
{
    QVariant value;
    if (true == readConfig(m_globalConfig, group, key, value)) {
        QStringList boards = value.toString().split(",");
        Q_FOREACH (const QString &board, boards) {
            if (m_hardwareInfo.contains(board, Qt::CaseInsensitive) && !board.isEmpty()) {
                return true;
            }
        }
    }
    return false;
}

bool UpmCustomDefaultGsettings::readConfig(
        QSettings *settings,
        const QString &group,
        const QString &key,
        QVariant &value)
{
    if (nullptr == settings) {
        return false;
    }
    settings->beginGroup(group);
    value = settings->value(key, UPM_NO_THIS_KEY);
    settings->endGroup();

    if (UPM_NO_THIS_KEY == value.toString()) {
        return false;
    }
    return true;
}

bool UpmCustomDefaultGsettings::getExtendConfigFile(QString &configFilePath)
{
    QDir extendConfigDir(UPM_EXTEND_GSETTINGS_CONFIG_PATH);

    if (true == extendConfigDir.exists()) {
        QStringList extendConfigs = extendConfigDir.entryList(QDir::Files | QDir::NoDotAndDotDot);
        Q_FOREACH (const QString &extendConfig, extendConfigs) {
            QStringList fileName = extendConfig.split(".");
            if (m_hardwareInfo.contains(fileName[0], Qt::CaseInsensitive)) {
                configFilePath = QString(UPM_EXTEND_GSETTINGS_CONFIG_PATH) + extendConfig;
                return true;
            }
        }
    }
    return false;
}

bool UpmCustomDefaultGsettings::readAll(const QString &filePath, QString &content)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "read file error :" << filePath;
        return false;
    }

    content = file.readAll();
    file.close();

    return true;
}
