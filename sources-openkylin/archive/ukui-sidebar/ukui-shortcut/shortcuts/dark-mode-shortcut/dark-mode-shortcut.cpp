/*
 * Copyright (C) 2025, KylinSoft Co., Ltd.
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
 * Authors: an-yongtai <anyongtai@kylinos.cn>
 *
 */

#include "dark-mode-shortcut.h"
#include <QDebug>
#include <QIcon>
#include <QFile>

#define SETTINGS_UKUI_STYLE      "org.ukui.style"
#define SETTINGS_GTK_STYLE       "org.mate.interface"
#define UKUI_DARKMODE_KEY        "styleName"
#define GTK_DARKMODE_KEY         "gtkTheme"
#define UKUI_DARKMODE_SYMBOLIC   "ukui-darkmode-24-symbolic"
#define UKUI_DARKMODE_LOTTIE     "ukui-darkmode-24-symbolic-motion.lottie"
#define THEME_LOTTIE_PATH        "/usr/share/icons/ukui-motion-icons"

using namespace UkuiShortcut;
UkuiShortcut::DarkModeShortcut::DarkModeShortcut(QObject *parent) : Shortcut(parent)
{
    m_currentStatus.setName(tr("Dark Mode"));
    m_currentStatus.setIcon(QFile::exists(THEME_LOTTIE_PATH "/" UKUI_DARKMODE_LOTTIE)
                            ? UKUI_DARKMODE_LOTTIE
                            : (QIcon::fromTheme(UKUI_DARKMODE_SYMBOLIC).isNull()
                               ? ":/darkmodeShortcut/ukui-dark.svg" : UKUI_DARKMODE_SYMBOLIC));
    m_currentStatus.setToolTip(tr("Dark Mode"));
    m_currentStatus.setColor(Color::ColorRole::BaseColor);
    initMetaData();
    QByteArray id(SETTINGS_UKUI_STYLE);
    if (QGSettings::isSchemaInstalled(id)) {
        m_gsettingsUkui = new QGSettings(id);
        if (m_gsettingsUkui->keys().contains(UKUI_DARKMODE_KEY, Qt::CaseInsensitive)) {
            m_isEnable = true;
        } else {
            qWarning() << "dark-mode-shortcut:" << "can't find key :" << UKUI_DARKMODE_KEY;
            m_isEnable = false;
        }
    } else {
        qWarning() << "dark-mode-shortcut:" << "can't find gsettings :" << SETTINGS_UKUI_STYLE;
        m_isEnable = false;
    }
    id = QByteArray(SETTINGS_GTK_STYLE);
    if (QGSettings::isSchemaInstalled(id)) {
        m_gsettingsGtk = new QGSettings(id);
        if (!m_gsettingsGtk->keys().contains(GTK_DARKMODE_KEY, Qt::CaseInsensitive)) {
            qWarning() << "dark-mode-shortcut:" << "can't find key :" << GTK_DARKMODE_KEY;
        }
    } else {
        qWarning() << "dark-mode-shortcut:" << "can't find gsettings :" << SETTINGS_GTK_STYLE;
    }
}

UkuiShortcut::DarkModeShortcut::~DarkModeShortcut()
{
    if (m_gsettingsUkui) {
        delete m_gsettingsUkui;
        m_gsettingsUkui = nullptr;
    }
    if (m_gsettingsGtk) {
        delete m_gsettingsGtk;
        m_gsettingsGtk = nullptr;
    }
}

void DarkModeShortcut::setIconButtonColor()
{
    if (m_gsettingsUkui->get(UKUI_DARKMODE_KEY).toString() == "ukui-dark") {
        m_currentStatus.setColor(Color::ColorRole::HighLight);
    } else if (m_gsettingsUkui->get(UKUI_DARKMODE_KEY).toString() == "ukui-light") {
        m_currentStatus.setColor(Color::ColorRole::BaseColor);
    }
}

void DarkModeShortcut::onQGSettingsChanged(const QString &key)
{
    if (key == UKUI_DARKMODE_KEY) {
        setIconButtonColor();
        Q_EMIT statusChanged(m_currentStatus);
    }
}

bool DarkModeShortcut::isEnable()
{
    return m_isEnable;
}

void DarkModeShortcut::setAddedState(bool added)
{
    if (m_isAdded != added) {
        m_isAdded = added;
        if (!m_gsettingsUkui) return;
        if (m_isAdded) {
            connect(m_gsettingsUkui, &QGSettings::changed, this, &DarkModeShortcut::onQGSettingsChanged);
            setIconButtonColor();
            Q_EMIT statusChanged(m_currentStatus);
        } else {
            disconnect(m_gsettingsUkui, &QGSettings::changed, this, &DarkModeShortcut::onQGSettingsChanged);
            m_currentStatus.setColor(Color::ColorRole::BaseColor);
            Q_EMIT statusChanged(m_currentStatus);
        }
    }
}

void DarkModeShortcut::active(PluginMetaType::Action action)
{
    if (action == PluginMetaType::Action::Click) {
        trigger();
    }
}

const StatusInfo DarkModeShortcut::currentStatus()
{
    return m_currentStatus;
}

QMap<PluginMetaType::SystemMode, PluginMetaData> DarkModeShortcut::pluginMetaData()
{
    return m_metaData;
}

void DarkModeShortcut::initMetaData()
{
    PluginMetaData pc {true, 8, PluginMetaType::PluginType::Icon};
    PluginMetaData tablet {true, 8, PluginMetaType::PluginType::Icon};
    m_metaData.insert(PluginMetaType::SystemMode::PC, pc);
    m_metaData.insert(PluginMetaType::SystemMode::Tablet, tablet);
}

void DarkModeShortcut::trigger()
{
    if (!m_isEnable || !m_isAdded) return;
    QString styleName;
    if (m_gsettingsUkui) {
        if (m_gsettingsUkui->keys().contains(UKUI_DARKMODE_KEY, Qt::CaseInsensitive)) {
            styleName = m_gsettingsUkui->get(UKUI_DARKMODE_KEY).toString();
            m_gsettingsUkui->set(UKUI_DARKMODE_KEY, styleName == "ukui-dark" ? "ukui-light" : "ukui-dark");
        }
    }
    if (m_gsettingsGtk) {
        if (m_gsettingsGtk->keys().contains(GTK_DARKMODE_KEY, Qt::CaseInsensitive)) {
            m_gsettingsGtk->set(GTK_DARKMODE_KEY, styleName == "ukui-dark" ? "ukui-white" : "ukui-black");
        }
    }
}

DarkModeShortcutPlugin::DarkModeShortcutPlugin(QObject *parent) : UkuiShortcutPlugin(parent)
{

}

QStringList DarkModeShortcutPlugin::translations()
{
    return {QStringLiteral("dark-mode-shortcut")};
}

Shortcut *DarkModeShortcutPlugin::createShortcut()
{
    return new DarkModeShortcut;
}
