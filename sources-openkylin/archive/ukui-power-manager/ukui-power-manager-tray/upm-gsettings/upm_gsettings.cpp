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
    initPowerGsettings();

    initPanelGsettings();

    initStyleGsettings();

    initControlCenterGsettings();
}

UpmGsettings::~UpmGsettings()
{
    delete m_powerGsettings;
    delete m_panelGsettings;
    delete m_styleGsettings;
    delete m_controlCenterGsettings;
}

UpmGsettings *UpmGsettings::self()
{
    return s_upmGsettings;
}

void UpmGsettings::dealGsettingsKeyChanged(const QString &key)
{
    if (true == m_qHashValue.contains(key)) {
        if (m_allPowerKey.contains(key)) {
            m_qHashValue[key] = m_powerGsettings->get(key);
        } else if (m_allPanelKey.contains(key)) {
            m_qHashValue[key] = m_panelGsettings->get(key);
        } else if (m_allStyleKey.contains(key)) {
            m_qHashValue[key] = m_styleGsettings->get(key);
        } else if (m_allControlCenterKey.contains(key)) {
            m_qHashValue[key] = m_controlCenterGsettings->get(key);
        }
        Q_EMIT (this->*m_qHashSignalsFuns[key])(m_qHashValue[key]);
        qDebug() << "key" << key << "changed";
    } else {
        qDebug() << "this key:" << key << "is not monitored!";
    }
}

void UpmGsettings::setGsettingsConfig(const QString &configName, QVariant value)
{
    if (true == m_qHashValue.contains(configName)) {
        m_qHashValue[configName] = value;
        if (m_allPowerKey.contains(configName)) {
            m_powerGsettings->set(configName, value);
        } else if (m_allPanelKey.contains(configName)) {
            m_panelGsettings->set(configName, value);
        } else if (m_allStyleKey.contains(configName)) {
            m_styleGsettings->set(configName, value);
        } else if (m_allControlCenterKey.contains(configName)) {
            m_controlCenterGsettings->set(configName, value);
        }
        qDebug() << "set key:" << configName << "value:" << value;
    } else {
        qInfo() << "this key:" << configName << "is not monitored!";
    }
}

bool UpmGsettings::getGsettingsConfig(const QString &configName, QVariant &value)
{
    qDebug() << "get key:" << configName;
    if (true == m_qHashValue.contains(configName)) {
        value = m_qHashValue[configName];
        return true;
    }
    qInfo() << "this key:" << configName << "is not monitored!";
    return false;
}

void UpmGsettings::addConfigMonitor(const QString &configName, pSignalFun signalFun)
{
    if (m_allPowerKey.contains(configName)) {
        m_qHashValue[configName] = m_powerGsettings->get(configName);
        m_qHashSignalsFuns[configName] = signalFun;
        qDebug() << "power gsetting:" << configName << "to be monitored!";
    } else if (m_allPanelKey.contains(configName)) {
        m_qHashValue[configName] = m_panelGsettings->get(configName);
        m_qHashSignalsFuns[configName] = signalFun;
        qDebug() << "panel gsetting:" << configName << "to be monitored!";
    } else if (m_allStyleKey.contains(configName)) {
        m_qHashValue[configName] = m_styleGsettings->get(configName);
        m_qHashSignalsFuns[configName] = signalFun;
        qDebug() << "style gsetting:" << configName << "to be monitored!";
    } else if (m_allControlCenterKey.contains(configName)) {
        m_qHashValue[configName] = m_controlCenterGsettings->get(configName);
        m_qHashSignalsFuns[configName] = signalFun;
        qDebug() << "control center gsetting:" << configName << "to be monitored!";
    } else {
        qInfo() << "without this key:" << configName;
    }
}

void UpmGsettings::initPowerGsettings()
{
    m_powerGsettings = new QGSettings(QByteArray(POWER_MANAGER_SETTINGS));
    m_allPowerKey = m_powerGsettings->keys();

    addConfigMonitor(GSETTINGS_KEY_POWER_POLICY_AC,
                     &UpmGsettings::powerPolicyAcChanged);
    addConfigMonitor(GSETTINGS_KEY_POWER_POLICY_BATTERY,
                     &UpmGsettings::powerPolicyBatteryChanged);
    addConfigMonitor(GSETTINGS_KEY_DISPLAY_LEFT_TIME,
                     &UpmGsettings::displayLeftTime);
    // 托盘电池图标阈值：低电量 / 极低电量（gsettings 键名为 kebab-case）
    addConfigMonitor(GSETTINGS_KEY_PERCENTAGE_LOW,
                     &UpmGsettings::percentageLowChanged);

    connect(m_powerGsettings, &QGSettings::changed, this, &UpmGsettings::dealGsettingsKeyChanged);
}

void UpmGsettings::initPanelGsettings()
{
    m_panelGsettings = new QGSettings(QByteArray(PANEL_SETTINGS));
    m_allPanelKey = m_panelGsettings->keys();

    addConfigMonitor(GSETTINGS_KEY_PANEL_SIZE,
                     &UpmGsettings::panelsizeChanged);
    addConfigMonitor(GSETTINGS_KEY_PANEL_POSITION,
                     &UpmGsettings::panelpositionChanged);

    connect(m_panelGsettings, &QGSettings::changed, this, &UpmGsettings::dealGsettingsKeyChanged);
}

void UpmGsettings::initStyleGsettings()
{
    m_styleGsettings = new QGSettings(QByteArray(STYLE_SETTINGS));
    m_allStyleKey = m_styleGsettings->keys();

    addConfigMonitor(GSETTINGS_KEY_SYSTEM_FONT_SIZE,
                     &UpmGsettings::systemFontSizeChanged);
    addConfigMonitor(GSETTINGS_KEY_STYLE_NAME,
                     &UpmGsettings::styleNameChanged);

    connect(m_styleGsettings, &QGSettings::changed, this, &UpmGsettings::dealGsettingsKeyChanged);
}

int UpmGsettings::percentageLowValue() const
{
    if (m_powerGsettings) {
        return m_powerGsettings->get(QStringLiteral(GSETTINGS_KEY_PERCENTAGE_LOW)).toInt();
    }
    return 20;
}

void UpmGsettings::initControlCenterGsettings()
{
    m_controlCenterGsettings = new QGSettings(QByteArray(CONTROL_CENTER_SETTINGS));
    m_allControlCenterKey = m_controlCenterGsettings->keys();

    addConfigMonitor(GSETTINGS_KEY_TRANSPARENCY_KEY,
                     &UpmGsettings::transparencyChanged);

    connect(m_controlCenterGsettings, &QGSettings::changed, this, &UpmGsettings::dealGsettingsKeyChanged);
}
