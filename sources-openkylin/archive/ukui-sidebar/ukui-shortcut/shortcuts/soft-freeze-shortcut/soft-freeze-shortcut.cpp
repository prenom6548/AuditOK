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

#include "soft-freeze-shortcut.h"
#include <QDebug>
#include <QCoreApplication>
#include <QTranslator>
#include <QIcon>

#define UKUI_SOFTFREEZE_SYMBOLIC     "ukui-soft-freeze-symbolic"
#define UKUI_PROCESS_MANAGER         "org.ukui.process-manager.soft-freeze-mode"
#define FREEZE_STATE                 "enabled"

using namespace UkuiShortcut;

UkuiShortcut::SoftFreezeShortcut::SoftFreezeShortcut(QObject *parent) : Shortcut(parent)
{
    initMetaData();

    QString iconName = QIcon::fromTheme(UKUI_SOFTFREEZE_SYMBOLIC).isNull() ? "://icon/ukui-soft-freeze-symbolic.svg" : UKUI_SOFTFREEZE_SYMBOLIC;
    m_currentStatus.setName(tr("Soft Freeze"));
    m_currentStatus.setIcon(iconName);
    m_currentStatus.setToolTip(tr("Soft Freeze"));
    m_currentStatus.setColor(Color::ColorRole::BaseColor);

    const QByteArray id(UKUI_PROCESS_MANAGER);

    if (QGSettings::isSchemaInstalled(id)) {
        m_gsettings = new QGSettings(id);

        if (m_gsettings->keys().contains(FREEZE_STATE, Qt::CaseInsensitive)) {
            m_state = m_gsettings->get(FREEZE_STATE).toBool();
            m_isEnable = true;
            m_currentStatus.setColor(m_state ? Color::ColorRole::HighLight : Color::ColorRole::BaseColor);
        } else {
            qWarning() << "SoftFreezeShortcut can't find key :" << FREEZE_STATE;
            m_isEnable = false;
        }

        connect(m_gsettings, &QGSettings::changed, this, [ & ](const QString & key) {
            if(key == FREEZE_STATE) {
                int state = m_gsettings->get(FREEZE_STATE).toBool();
                freezeStateChanged(state);
            }
        });
    } else {
        qWarning() << "SoftFreezeShortcut can't find gsettings :" << UKUI_PROCESS_MANAGER;
        m_isEnable = false;
    }
}

SoftFreezeShortcut::~SoftFreezeShortcut()
{

}

QString SoftFreezeShortcut::pluginId()
{
    return QLatin1String("SoftFreeze");
}

bool SoftFreezeShortcut::isEnable()
{
    //task 407405
    return false;
    // return m_isEnable;
}

void SoftFreezeShortcut::active(PluginMetaType::Action action)
{
    if(action == PluginMetaType::Action::Click) {
        if(m_gsettings && m_isEnable) {
            if(m_gsettings->keys().contains(FREEZE_STATE, Qt::CaseInsensitive)) {
                m_gsettings->set(FREEZE_STATE, m_state ? false : true);
            }
        }
    }
}

const StatusInfo SoftFreezeShortcut::currentStatus()
{
    return m_currentStatus;
}

QMap<PluginMetaType::SystemMode, PluginMetaData> SoftFreezeShortcut::pluginMetaData()
{
    return m_metaData;
}

void SoftFreezeShortcut::initMetaData()
{
    PluginMetaData pc {true, 7, PluginMetaType::PluginType::Icon};
    PluginMetaData tablet {false, 7, PluginMetaType::PluginType::Icon};
    m_metaData.insert(PluginMetaType::SystemMode::PC, pc);
    m_metaData.insert(PluginMetaType::SystemMode::Tablet, tablet);
}

void SoftFreezeShortcut::freezeStateChanged(bool state)
{
    if (m_state == state) {
        return;
    }

    m_currentStatus.setColor((state ? Color::ColorRole::HighLight : Color::ColorRole::BaseColor));
    m_state = state;
    Q_EMIT statusChanged(m_currentStatus);
}

SoftFreezeShortcutPlugin::SoftFreezeShortcutPlugin(QObject *parent) : UkuiShortcutPlugin(parent)
{

}

QStringList SoftFreezeShortcutPlugin::translations()
{
    return {QStringLiteral("soft-freeze-shortcut")};
}

Shortcut *SoftFreezeShortcutPlugin::createShortcut()
{
    return new SoftFreezeShortcut;
}
