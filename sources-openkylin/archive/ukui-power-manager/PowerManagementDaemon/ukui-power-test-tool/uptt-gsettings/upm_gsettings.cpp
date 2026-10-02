/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
#include "upm_gsettings.h"

#include <QDebug>

Q_GLOBAL_STATIC(UpmGsettings, s_upmGsettings)

UpmGsettings::UpmGsettings(QObject *parent) : QObject(parent)
{
    const QByteArray powerGsettingsID(POWER_MANAGER_SETTINGS);
    m_powerGsettings = new QGSettings(powerGsettingsID);
    m_allPowerKey = m_powerGsettings->keys();

    const QByteArray processGsettingsID(SCREEN_SAVER_SETTINGS);
    m_screensaverGsettings = new QGSettings(processGsettingsID);
    m_allScreensaverKey = m_screensaverGsettings->keys();

    const QByteArray usdAutoBrightnessGsettingsID(USD_AUTO_BRIGHTENSS_SETTINGS);
    m_usdAutoBrightnessGsettings = new QGSettings(usdAutoBrightnessGsettingsID);
    m_allUsdAutoBrightnessKey = m_usdAutoBrightnessGsettings->keys();

    initGsettingsConfig();
}

UpmGsettings::~UpmGsettings()
{
    delete m_powerGsettings;
    delete m_screensaverGsettings;
    delete m_usdAutoBrightnessGsettings;
}

UpmGsettings *UpmGsettings::self()
{
    return s_upmGsettings;
}

void UpmGsettings::setGsettingsConfig(const QString &configName, QVariant value)
{
    if (true == m_qHashValue.contains(configName)) {
        m_qHashValue[configName] = value;
        if (m_allPowerKey.contains(configName)) {
            m_powerGsettings->set(configName, value);
        } else if (m_allScreensaverKey.contains(configName)) {
            m_screensaverGsettings->set(configName, value);
        } else if (m_allUsdAutoBrightnessKey.contains(configName)) {
            m_usdAutoBrightnessGsettings->set(configName, value);
        }
    }
}

bool UpmGsettings::getGsettingsConfig(const QString &configName, QVariant &value)
{
    if (true == m_qHashValue.contains(configName)) {
        value = m_qHashValue[configName];
        return true;
    }
    return false;
}

void UpmGsettings::resetGsettingsConfig(const QString &configName)
{
    if (true == m_qHashValue.contains(configName)) {
        if (m_allPowerKey.contains(configName)) {
            m_powerGsettings->reset(configName);
        } else if (m_allScreensaverKey.contains(configName)) {
            m_screensaverGsettings->reset(configName);
        } else if (m_allUsdAutoBrightnessKey.contains(configName)) {
            m_usdAutoBrightnessGsettings->reset(configName);
        }
    }
}

void UpmGsettings::addConfigMonitor(const QString &configName, pSignalFun signalFun)
{
    if (m_allPowerKey.contains(configName)) {
        m_qHashValue[configName] = m_powerGsettings->get(configName);
    } else if (m_allScreensaverKey.contains(configName)) {
        m_qHashValue[configName] = m_screensaverGsettings->get(configName);
    } else if (m_allUsdAutoBrightnessKey.contains(configName)) {
        m_qHashValue[configName] = m_usdAutoBrightnessGsettings->get(configName);
    } else {
        ;
    }
}

void UpmGsettings::initGsettingsConfig()
{
    addConfigMonitor(GSETTINGS_KEY_POWER_POLICY_BATTERY, NULL);

    addConfigMonitor(GSETTINGS_KEY_SLEEP_DISPLAY_BATTERY, NULL);

    addConfigMonitor(GSETTINGS_KEY_SLEEP_COMPUTER_BATTERY, NULL);

    addConfigMonitor(GSETTINGS_KEY_IDLE_BRIGHTNESS, NULL);

    addConfigMonitor(GSETTINGS_KEY_IDLE_POWER_POLICY_AUTO, NULL);

    addConfigMonitor(GSETTINGS_KEY_PERCENTAGE_ACTION, NULL);

    addConfigMonitor(GSETTINGS_KEY_ACTION_CRITICAL_BATTERY, NULL);

    addConfigMonitor(GSETTINGS_KEY_BRIGHTNESS_AC, NULL);

    addConfigMonitor(GSETTINGS_KEY_IDLE_DELAY, NULL);

    addConfigMonitor(GSETTINGS_KEY_AUTO_BRIGHTENSS, NULL);
}
