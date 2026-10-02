/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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

#include "flight-mode-shortcut.h"
#include <QDebug>
#include <QTranslator>
#include <QApplication>
#include <QFile>

#define USD_PLUGIN_MEDIA_KEY_STATE       "org.ukui.SettingsDaemon.plugins.media-keys-state"
#define RFKILL_STATE                     "rfkillState"
#define FLIGHT_MODE_ICON                 "airplane-mode-24-symbolic"
#define FLIGHT_MODE_LOTTIE               "airplane-mode-24-symbolic-motion.lottie"
#define THEME_LOTTIE_PATH                "/usr/share/icons/ukui-motion-icons"
using namespace UkuiShortcut;
FlightModeShortcut::FlightModeShortcut()
{
    const QByteArray id(USD_PLUGIN_MEDIA_KEY_STATE);

    initMetaData();

    if(QGSettings::isSchemaInstalled(id)) {
        m_gsettings = new QGSettings(id);
        if(m_gsettings->keys().contains(RFKILL_STATE, Qt::CaseInsensitive)) {
            int state = m_gsettings->get(RFKILL_STATE).toInt();
            if(state == -1) {
                m_isEnable = false;
            } else {
                m_isEnable = true;
                m_state = state;
                m_statusInfo.setColor(m_state == 0? Color::ColorRole::BaseColor: Color::ColorRole::HighLight);
            }
        } else {
            qWarning() << "FlightModeShortcut:" << "can't find key :" << RFKILL_STATE;
            m_isEnable = false;
        }

        connect(m_gsettings, &QGSettings::changed, this, [ & ](const QString & key) {
            if(key == RFKILL_STATE) {
                int state = m_gsettings->get(RFKILL_STATE).toInt();
                stateChanged(state);
                qDebug() << "FlightModeShortcut value changed:" << state;
            }
        });
        m_statusInfo.setIcon(QFile::exists(THEME_LOTTIE_PATH "/" FLIGHT_MODE_LOTTIE) ? FLIGHT_MODE_LOTTIE : FLIGHT_MODE_ICON);
        m_statusInfo.setName(tr("Flight Mode"));
        m_statusInfo.setToolTip(tr("Flight Mode"));
    } else {
        qWarning() << "FlightModeShortcut:" << "can't find gsettings :" << USD_PLUGIN_MEDIA_KEY_STATE;
        m_isEnable = false;
    }
}

void FlightModeShortcut::active(PluginMetaType::Action action)
{
    if(action == PluginMetaType::Action::Click) {
        if(m_gsettings && m_isEnable) {
            if(m_gsettings->keys().contains(RFKILL_STATE, Qt::CaseInsensitive)) {
                m_gsettings->set(RFKILL_STATE, m_state == 0? 1: 0);
            }
        }
    }
}

const StatusInfo FlightModeShortcut::currentStatus()
{
    return m_statusInfo;
}

bool FlightModeShortcut::isEnable()
{
    return m_isEnable;
}

/**
 * @brief FlightModeShortcut::stateChanged
 * @param state 0-flight mode off, 1-flight-mode on, -1-flight mode not being supported
 */
void FlightModeShortcut::stateChanged(int state)
{
    if(m_state == state) {
        return;
    }
    if(state == 0 || state == 1) {
        m_statusInfo.setColor(state == 0? Color::ColorRole::BaseColor: Color::ColorRole::HighLight);
        if(m_state == -1) {
            m_isEnable = true;
            Q_EMIT enableStatusChanged(m_isEnable);
        }
        m_state = state;
        Q_EMIT statusChanged(m_statusInfo);

    } else if(state == -1) {
        m_state = state;
        m_isEnable = false;
        Q_EMIT enableStatusChanged(m_isEnable);
    }
}

QMap<PluginMetaType::SystemMode, PluginMetaData> FlightModeShortcut::pluginMetaData()
{
    return m_metaData;
}

void FlightModeShortcut::initMetaData()
{
    PluginMetaData pc {true, 2, PluginMetaType::PluginType::Icon};
    PluginMetaData tablet {true, 2, PluginMetaType::PluginType::Icon};
    m_metaData.insert(PluginMetaType::SystemMode::PC, pc);
    m_metaData.insert(PluginMetaType::SystemMode::Tablet, tablet);
}

FlightModeShortcutPlugin::FlightModeShortcutPlugin(QObject *parent) : UkuiShortcutPlugin(parent)
{

}

QStringList FlightModeShortcutPlugin::translations()
{
    return {QStringLiteral("flight-mode-shortcut")};
}

Shortcut *FlightModeShortcutPlugin::createShortcut()
{
    return new FlightModeShortcut;
}
