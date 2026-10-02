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

#include "night-mode-shortcut.h"
#include <QDebug>
#include <QApplication>
#include <QTranslator>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <private/qtx11extras_p.h>
#else
#include <QX11Info>
#endif
#define SETTINGS_DAEMON_PLUGIN_COLOR          "org.ukui.SettingsDaemon.plugins.color"
#define DARKMODE_KEY         "darkMode"
#define UKUI_NIGHTMODE_SYMBOLIC     "ukui-nightmode-symbolic"


using namespace UkuiShortcut;

UkuiShortcut::NightModeShortcut::NightModeShortcut(QObject *parent) : Shortcut(parent)
{
    m_currentStatus.setName(tr("Night mode"));
    m_currentStatus.setIcon(UKUI_NIGHTMODE_SYMBOLIC);
    m_currentStatus.setToolTip("Night Mode");
    m_currentStatus.setColor(Color::ColorRole::BaseColor);

    initMetaData();

    // fix: issue: #I8Q0DU【wayland】【wlcom】【侧边栏】缺少夜间模式
    //if (!QX11Info::isPlatformX11()) {
    //    // wayland环境暂不支持夜间模式
    //    m_isEnable = false;
    //    return;
    //}

    const QByteArray id(SETTINGS_DAEMON_PLUGIN_COLOR);
    if(QGSettings::isSchemaInstalled(id)) {
        m_gsettings = new QGSettings(id);

        if(m_gsettings->keys().contains(DARKMODE_KEY, Qt::CaseInsensitive)) {
            if(m_gsettings->get(DARKMODE_KEY).toBool()) {
                m_currentStatus.setColor(Color::ColorRole::HighLight);
            } else {
                m_currentStatus.setColor(Color::ColorRole::BaseColor);
            }
            m_isEnable = true;
        } else {
            qWarning() << "NightModeShortcut:" << "can't find key :" << DARKMODE_KEY;
            m_isEnable = false;
        }

        connect(m_gsettings, &QGSettings::changed, this, [ = ](const QString & key) {
            if(key == DARKMODE_KEY) {
                if(m_gsettings->get(DARKMODE_KEY).toBool()) {
                    m_currentStatus.setColor(Color::ColorRole::HighLight);
                } else {
                    m_currentStatus.setColor(Color::ColorRole::BaseColor);
                }
                Q_EMIT statusChanged(m_currentStatus);
            }
        });

    } else {
        qWarning() << "NightModeShortcut:" << "can't find gsettings :" << SETTINGS_DAEMON_PLUGIN_COLOR;
        m_isEnable = false;
    }
}

UkuiShortcut::NightModeShortcut::~NightModeShortcut()
{
    if(m_gsettings) {
        delete m_gsettings;
        m_gsettings = nullptr;
    }
}

bool NightModeShortcut::isEnable()
{
    return m_isEnable;
}

void NightModeShortcut::active(PluginMetaType::Action action)
{
    if (action == PluginMetaType::Action::Click) {
        if(m_gsettings) {
            if(m_gsettings->keys().contains(DARKMODE_KEY, Qt::CaseInsensitive)) {
                m_gsettings->set(DARKMODE_KEY, !m_gsettings->get(DARKMODE_KEY).toBool());
            }
        }
    }
}

const StatusInfo NightModeShortcut::currentStatus()
{
    return m_currentStatus;
}

QMap<PluginMetaType::SystemMode, PluginMetaData> NightModeShortcut::pluginMetaData()
{
    return m_metaData;
}

void NightModeShortcut::initMetaData()
{
    PluginMetaData pc {true, 3, PluginMetaType::PluginType::Icon};
    PluginMetaData tablet {true, 3, PluginMetaType::PluginType::Icon};
    m_metaData.insert(PluginMetaType::SystemMode::PC, pc);
    m_metaData.insert(PluginMetaType::SystemMode::Tablet, tablet);
}

NightModeShortcutPlugin::NightModeShortcutPlugin(QObject *parent) : UkuiShortcutPlugin(parent)
{

}

QStringList NightModeShortcutPlugin::translations()
{
    return {QStringLiteral("night-mode-shortcut")};
}

Shortcut *NightModeShortcutPlugin::createShortcut()
{
    return new NightModeShortcut;
}
