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

#include "multi-screen-shortcut.h"
#include <QProcess>
#include <QDebug>
#include <QApplication>
#include <QTranslator>
#include <QFile>

#define PROJECTION_ICON     "ukui-projection-24-symbolic"
#define PROJECTION_LOTTIE   "ukui-projection-24-symbolic-motion.lottie"
#define THEME_LOTTIE_PATH   "/usr/share/icons/ukui-motion-icons"
using namespace UkuiShortcut;
MultiScreenShortcut::MultiScreenShortcut()
{
    m_statusInfo.setName(tr("Multi-Screen"));
    m_statusInfo.setToolTip(tr("Multi-Screen"));
    m_statusInfo.setIcon(QFile::exists(THEME_LOTTIE_PATH "/" PROJECTION_LOTTIE) ? PROJECTION_LOTTIE : PROJECTION_ICON);
    m_statusInfo.setColor(Color::ColorRole::BaseColor);
    initMetaData();
}

void UkuiShortcut::MultiScreenShortcut::active(PluginMetaType::Action action)
{
    if(action == PluginMetaType::Action::Click) {
        //命令行开始执行
        if (!QProcess::startDetached("ukydisplayswitch")) {
            qWarning()<<"MultiScreenShortcut: start ukydisplayswitch failed";
        }
    }
}

const UkuiShortcut::StatusInfo UkuiShortcut::MultiScreenShortcut::currentStatus()
{
    return m_statusInfo;
}

QMap<PluginMetaType::SystemMode, PluginMetaData> MultiScreenShortcut::pluginMetaData()
{
    return m_metaData;
}

void MultiScreenShortcut::initMetaData()
{
    PluginMetaData pc {true, 4, PluginMetaType::PluginType::Icon};
    PluginMetaData tablet {true, 4, PluginMetaType::PluginType::Icon};
    m_metaData.insert(PluginMetaType::SystemMode::PC, pc);
    m_metaData.insert(PluginMetaType::SystemMode::Tablet, tablet);
}

MultiScreenShortcutPlugin::MultiScreenShortcutPlugin(QObject *parent) : UkuiShortcutPlugin(parent)
{

}

QStringList MultiScreenShortcutPlugin::translations()
{
    return {QStringLiteral("multi-screen-shortcut")};
}

Shortcut *MultiScreenShortcutPlugin::createShortcut()
{
    return new MultiScreenShortcut;
}
