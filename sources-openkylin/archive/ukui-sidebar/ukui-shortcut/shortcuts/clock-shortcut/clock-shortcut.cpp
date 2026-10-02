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

#include "clock-shortcut.h"
#include <QDebug>
#include <QColor>
#include <QProcess>
#include <QDBusInterface>
#include <QFile>
#include <QIcon>

#define KYLIN_APP_MANAGER_NAME         "com.kylin.ProcessManager"
#define KYLIN_APP_MANAGER_PATH         "/com/kylin/ProcessManager/AppLauncher"
#define KYLIN_APP_MANAGER_INTERFACE    "com.kylin.ProcessManager.AppLauncher"
#define UKUI_CLOCK_NAME  "ukui-clock"
#define UKUI_CLOCK_SYMBOLIC            "alarm-24-symbolic"
#define UKUI_CLOCK_LOTTIE              "alarm-24-symbolic-motion.lottie"
#define THEME_LOTTIE_PATH              "/usr/share/icons/ukui-motion-icons"

using namespace UkuiShortcut;
ClockShortcut::ClockShortcut(QObject *parent) : Shortcut(parent)
{
    m_currentStatusInfo.setColor(Color::ColorRole::BaseColor);
    m_currentStatusInfo.setName(tr("Clock"));
    m_currentStatusInfo.setIcon(QFile::exists(THEME_LOTTIE_PATH "/" UKUI_CLOCK_LOTTIE)
                                ? UKUI_CLOCK_LOTTIE
                                : (QIcon::fromTheme(UKUI_CLOCK_SYMBOLIC).isNull()
                                   ? ":/clockShortcut/ukui-clock.svg" : UKUI_CLOCK_SYMBOLIC));
    m_currentStatusInfo.setToolTip(tr("Clock"));

    m_applicationInfo = new ApplicationInfo;
    m_appPath = getDesktop(UKUI_CLOCK_NAME);
    m_isEnable = !(m_appPath == "");

    connect(m_applicationInfo, &ApplicationInfo::appDBItems2BDelete, this, &ClockShortcut::onAppUninstalled);
    connect(m_applicationInfo, &ApplicationInfo::appDBItems2BAdd, this, &ClockShortcut::onAppInstalled);
    initMetaData();
}

ClockShortcut::~ClockShortcut()
{
    if (m_applicationInfo) {
        delete m_applicationInfo;
        m_applicationInfo = nullptr;
    }
}

QString ClockShortcut::pluginId()
{
    return QStringLiteral("Clock");
}

void ClockShortcut::active(PluginMetaType::Action action)
{
    if (action == PluginMetaType::Action::Click) {
        LaunchApp();
    }
}

const StatusInfo ClockShortcut::currentStatus()
{
    return m_currentStatusInfo;
}

bool ClockShortcut::isEnable()
{
    return m_isEnable;
}

void ClockShortcut::setAddedState(bool added)
{
    if (m_isAdded != added) {
        m_isAdded = added;
    }
}

void ClockShortcut::LaunchApp()
{
    if (!m_isEnable || !m_isAdded) return;
    QDBusMessage message = QDBusMessage::createMethodCall(KYLIN_APP_MANAGER_NAME,
                                                          KYLIN_APP_MANAGER_PATH,
                                                          KYLIN_APP_MANAGER_INTERFACE,
                                                          "LaunchApp");
    message << m_appPath;
    auto watcher = new QDBusPendingCallWatcher(QDBusConnection::sessionBus().asyncCall(message),this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [ = ] (QDBusPendingCallWatcher *self) {
        if (self) {
            if (self->isError()) {
                qDebug()<<"clock-shortcut: use dbus failed" << self->error().message();
                if (!QProcess::startDetached("ukui-clock")) {
                    qDebug()<<"clock-shortcut: ukui-clock is failed";
                }
            }
            self->deleteLater();
        }
    });
}

QMap<PluginMetaType::SystemMode, PluginMetaData> ClockShortcut::pluginMetaData()
{
    return m_metaData;
}

void ClockShortcut::initMetaData()
{
    PluginMetaData pc {true, 10, PluginMetaType::PluginType::Icon};
    pc.setPreAction(PluginMetaType::Hide);
    PluginMetaData tablet {true, 10, PluginMetaType::PluginType::Icon};
    tablet.setPreAction(PluginMetaType::Hide);
    m_metaData.insert(PluginMetaType::SystemMode::PC, pc);
    m_metaData.insert(PluginMetaType::SystemMode::Tablet, tablet);
}

void ClockShortcut::onAppUninstalled(const QStringList &desktopFiles)
{
    if (std::any_of(desktopFiles.begin(), desktopFiles.end(),
                    [&](const QString &str) {return str.contains(UKUI_CLOCK_NAME);})) {
        checkAppInstallState();
    }
}

void ClockShortcut::onAppInstalled(const QStringList &desktopFiles)
{
    if (std::any_of(desktopFiles.begin(), desktopFiles.end(),
                    [&](const QString &str) {return str.contains(UKUI_CLOCK_NAME);})) {
        checkAppInstallState();
    }
}

QString ClockShortcut::getDesktop(QString id)
{
    QString path = "";
    m_applicationInfo->desktopFilePathFromName(id, path);
    return path;
}

void ClockShortcut::checkAppInstallState()
{
    m_appPath = getDesktop(UKUI_CLOCK_NAME);
    bool enable = !(m_appPath == "");
    if (m_isEnable == enable) return;
    m_isEnable = enable;
    Q_EMIT enableStatusChanged(m_isEnable);
}

ClockShortcutPlugin::ClockShortcutPlugin(QObject *parent) : UkuiShortcutPlugin(parent)
{

}

QStringList ClockShortcutPlugin::translations()
{
    return {QStringLiteral("clock-shortcut")};
}

Shortcut *ClockShortcutPlugin::createShortcut()
{
    return new ClockShortcut;
}
