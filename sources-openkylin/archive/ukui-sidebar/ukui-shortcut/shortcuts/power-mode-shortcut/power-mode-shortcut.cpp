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

#include "power-mode-shortcut.h"
#include <QDBusConnection>
#include <QDBusReply>
#include <QDebug>
#include <QString>
#include <QStringList>
#include <QApplication>
#include <QTranslator>
#define POWER_BUS_NAME "org.ukui.upower"
#define POWER_BUS_PATH "/upower/PowerManager"
#define POWER_BUS_INTERFACE "org.ukui.powerManager"
#define POWER_CONFIG_CHANGED_SIGNAL "PowerConfigChanged"
#define GET_BATTERYSAVER_METHOD "GetBatterySaver"
#define SET_BATTERYSAVER_METHOD "SetBatterySaver"

using namespace UkuiShortcut;

PowerModeShortcut::PowerModeShortcut()
{
    initMetaData();
    m_powerManagerIface = new QDBusInterface(POWER_BUS_NAME, POWER_BUS_PATH, POWER_BUS_INTERFACE, QDBusConnection::sessionBus(), this);
    if(!m_powerManagerIface->isValid()) {
        qWarning() << "PowerModeShortcut error:" << m_powerManagerIface->lastError();
        m_isEnable = false;
        return;
    }

    QDBusReply<int> reply = m_powerManagerIface->call(GET_BATTERYSAVER_METHOD);
    if(!reply.isValid()) {
        qWarning() << "PowerModeShortcut call " << GET_BATTERYSAVER_METHOD << "failed" << reply.value();
        m_isEnable = false;
        return;
    } else {
        setState(reply.value());
    }

    if(!QDBusConnection::sessionBus().connect(POWER_BUS_NAME,
                                              POWER_BUS_PATH,
                                              POWER_BUS_INTERFACE,
                                              POWER_CONFIG_CHANGED_SIGNAL,
                                              this,
                                              SLOT(stateChangedSlot(QStringList)))) {
        qWarning() << "PowerModeShortcut connect " << POWER_CONFIG_CHANGED_SIGNAL << "failed";
    }

    m_currentStatus.setName(tr("Power-Saving Mode"));
    m_currentStatus.setIcon("ukui-eco-24-symbolic");
    m_currentStatus.setToolTip(tr("Power-Saving Mode"));
}

PowerModeShortcut::~PowerModeShortcut()
{
}

void PowerModeShortcut::active(PluginMetaType::Action action)
{
    switch (action) {
    case PluginMetaType::Action::Click:
        if(!m_powerManagerIface) {
            return;
        }
        if(m_powerState == 0 || m_powerState ==1) {
            int target_state = (m_powerState==0)?(1):(0);
            m_powerManagerIface->call(SET_BATTERYSAVER_METHOD, target_state);
            setState(target_state);
        }
        break;
    default:
        break;
    }
}

const StatusInfo PowerModeShortcut::currentStatus()
{
    return m_currentStatus;
}

void PowerModeShortcut::stateChangedSlot(QStringList keys)
{
    if (!keys.contains("batterySaver")) return;
    QDBusReply<int> reply = m_powerManagerIface->call(GET_BATTERYSAVER_METHOD);
    if (reply.isValid()) {
        setState(reply.value());
    }
}

void PowerModeShortcut::setState(const int state)
{
    m_powerState = state;
    bool enable = state != 3;
    if(m_powerState == 0) {
        m_currentStatus.setColor(Color::ColorRole::BaseColor);
        m_currentStatus.setDisable(false);
    } else if (m_powerState == 1) {
        m_currentStatus.setColor(Color::ColorRole::HighLight);
        m_currentStatus.setDisable(false);
    } else if (m_powerState == 2) {
        m_currentStatus.setColor(Color::ColorRole::Disable);
        m_currentStatus.setDisable(true);
    }

    if (m_isEnable != enable) {
        m_isEnable = enable;
        Q_EMIT enableStatusChanged(m_isEnable);
    }
    Q_EMIT statusChanged(m_currentStatus);
}

QMap<PluginMetaType::SystemMode, PluginMetaData> PowerModeShortcut::pluginMetaData()
{
    return m_metaData;
}

void PowerModeShortcut::initMetaData()
{
    PluginMetaData pc {true, 5, PluginMetaType::PluginType::Icon};
    PluginMetaData tablet {true, 5, PluginMetaType::PluginType::Icon};
    m_metaData.insert(PluginMetaType::SystemMode::PC, pc);
    m_metaData.insert(PluginMetaType::SystemMode::Tablet, tablet);
}

PowerModeShortcutPlugin::PowerModeShortcutPlugin(QObject *parent) : UkuiShortcutPlugin(parent)
{

}

QStringList PowerModeShortcutPlugin::translations()
{
    return {QStringLiteral("power-mode-shortcut")};
}

Shortcut *PowerModeShortcutPlugin::createShortcut()
{
    return new PowerModeShortcut;
}
