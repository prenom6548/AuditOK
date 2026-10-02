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

#include "system-setting-shortcut.h"

#include <QDebug>
#include <QColor>
#include <QDBusReply>
#include <QProcess>
#include <QDBusInterface>
#include <QApplication>
#include <QTranslator>

#define POWER_MANAGER_SETTINGS       "org.ukui.power-manager"

using namespace UkuiShortcut;
SystemSettingShortcut::SystemSettingShortcut(QObject *parent) : Shortcut(parent)
{
    m_currentStatusInfo.setColor(Color::ColorRole::BaseColor);//当前状态信息初始化
    m_currentStatusInfo.setName(tr("Settings"));
    m_currentStatusInfo.setIcon("system-settings-symbolic");
    m_currentStatusInfo.setToolTip(tr("Settings"));
    m_isEnable = false;

    initMetaData();
}

SystemSettingShortcut::~SystemSettingShortcut()
{

}


QString SystemSettingShortcut::pluginId()
{
    return QStringLiteral("SystemSetting");
}

void SystemSettingShortcut::active(PluginMetaType::Action action)
{
    if (action == PluginMetaType::Action::Click) {
        qDebug() <<"Shortcut::Action::Click";
        trigger();
    }
}

const StatusInfo SystemSettingShortcut::currentStatus()
{
    return m_currentStatusInfo;
}

bool SystemSettingShortcut::isEnable()
{
    return m_isEnable;
}

bool SystemSettingShortcut::LaunchApp(QString desktopFile)
{

    QDBusInterface m_appManagerDbusInterface(KYLIN_APP_MANAGER_NAME,
                                             KYLIN_APP_MANAGER_PATH,
                                             KYLIN_APP_MANAGER_INTERFACE,
                                             QDBusConnection::sessionBus());//局部变量

    if (!m_appManagerDbusInterface.isValid()) {
        qWarning()<<"m_appManagerDbusInterface init error";
        return false;
    } else {
        QDBusReply<bool> reply =m_appManagerDbusInterface.call("LaunchApp",desktopFile);
        return reply;
    }
}

void SystemSettingShortcut::trigger()
{
    if (!LaunchApp("/usr/share/applications/ukui-control-center.desktop")) {
        //命令行开始执行
        if (!QProcess::startDetached("ukui-control-center")) {
            qWarning()<<"ukui-control-center is failed";
        }
    }
}

QMap<PluginMetaType::SystemMode, PluginMetaData> SystemSettingShortcut::pluginMetaData()
{
    return m_metaData;
}

void SystemSettingShortcut::initMetaData()
{
    PluginMetaData pc {true, -1, PluginMetaType::PluginType::Icon};
    PluginMetaData tablet {true, -1, PluginMetaType::PluginType::Icon};
    m_metaData.insert(PluginMetaType::SystemMode::PC, pc);
    m_metaData.insert(PluginMetaType::SystemMode::Tablet, tablet);
}

SystemSettingShortcutPlugin::SystemSettingShortcutPlugin(QObject *parent) : UkuiShortcutPlugin(parent)
{

}

QStringList SystemSettingShortcutPlugin::translations()
{
    return {QStringLiteral("system-setting-shortcut")};
}

Shortcut *SystemSettingShortcutPlugin::createShortcut()
{
    return new SystemSettingShortcut;
}
