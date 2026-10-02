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

#include "screenshot-shortcut.h"
#include <QDebug>
#include <QApplication>
#include <QTranslator>
#include <QProcess>
#include <QDBusReply>
#include <QFile>

#define UKUI_SCREENSHOT_SYMBOLIC     "screenshot-app-24-symbolic"
#define UKUI_SCREENSHOT_LOTTIE       "screenshot-app-24-symbolic-motion.lottie"
#define THEME_LOTTIE_PATH            "/usr/share/icons/ukui-motion-icons"

using namespace UkuiShortcut;

UkuiShortcut::ScreenShotShortcut::ScreenShotShortcut(QObject *parent) : Shortcut(parent)
{
    m_currentStatus.setName(tr("Screen Shot"));
    m_currentStatus.setIcon(QFile::exists(THEME_LOTTIE_PATH "/" UKUI_SCREENSHOT_LOTTIE) ? UKUI_SCREENSHOT_LOTTIE : UKUI_SCREENSHOT_SYMBOLIC);
    m_currentStatus.setToolTip(tr("Screen Shot"));
    m_currentStatus.setColor(Color::ColorRole::BaseColor);
    m_applicationInfo = new UkuiSearch::ApplicationInfo;
    m_applicationInfo->desktopFilePathFromName(KYLIN_SCREENSHOT, m_appPath);
    m_isEnable = !(m_appPath.isEmpty());
    connect(m_applicationInfo, &UkuiSearch::ApplicationInfo::appDBItems2BDelete, this, &ScreenShotShortcut::onAppInstallStatusChanged);
    connect(m_applicationInfo, &UkuiSearch::ApplicationInfo::appDBItems2BAdd, this, &ScreenShotShortcut::onAppInstallStatusChanged);
    initMetaData();
}

ScreenShotShortcut::~ScreenShotShortcut()
{
    if (m_applicationInfo) {
        delete m_applicationInfo;
        m_applicationInfo = nullptr;
    }
}

void ScreenShotShortcut::kyscreenshotbegin()
{
    if(!m_isEnable) {
        return;
    } else {
        QDBusMessage message = QDBusMessage::createMethodCall(KYLIN_APP_MANAGER_NAME,
                                                              KYLIN_APP_MANAGER_PATH,
                                                              KYLIN_APP_MANAGER_INTERFACE,
                                                              "LaunchAppWithArguments");
        QStringList args{};
        message << m_appPath << args;

        auto watcher = new QDBusPendingCallWatcher(QDBusConnection::sessionBus().asyncCall(message), this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this, [args] (QDBusPendingCallWatcher *self) {
            if (self) {
                if (self->isError()) {
                    qDebug() << "use dbus falsed" << self->error().message();
                    QProcess::startDetached("kylin-screenshot", args);
                }
                self->deleteLater();
            }
        });
    }
}


QString ScreenShotShortcut::pluginId()
{
    return QLatin1String("ScreenShot");
}

bool ScreenShotShortcut::isEnable()
{
    return m_isEnable;
}

void ScreenShotShortcut::active(PluginMetaType::Action action)
{
    switch (action) {
        default:
        case PluginMetaType::Action::Click:
            //qDebug() << "NightModeShortcut 点击事件" << "size";
            kyscreenshotbegin();
            break;
        case PluginMetaType::Action::LongClick:
            //qDebug() << "长按事件";
            break;
        case PluginMetaType::Action::MenuRequest:
            //qDebug() << "菜单事件";
            break;
    }
}

const StatusInfo ScreenShotShortcut::currentStatus()
{
    return m_currentStatus;
}

QMap<PluginMetaType::SystemMode, PluginMetaData> ScreenShotShortcut::pluginMetaData()
{
    return m_metaData;
}

void ScreenShotShortcut::initMetaData()
{
    PluginMetaData pc {true, 6, PluginMetaType::PluginType::Icon};
    pc.setPreAction(PluginMetaType::Hide);
    PluginMetaData tablet {true, 6, PluginMetaType::PluginType::Icon};
    tablet.setPreAction(PluginMetaType::Hide);

    m_metaData.insert(PluginMetaType::SystemMode::PC, pc);
    m_metaData.insert(PluginMetaType::SystemMode::Tablet, tablet);
}

void ScreenShotShortcut::onAppInstallStatusChanged(const QStringList &desktopFiles)
{
    if (std::any_of(desktopFiles.begin(), desktopFiles.end(),
                    [&](const QString &str) {return str.contains(KYLIN_SCREENSHOT);})) {
        checkAppInstallState();
    }
}

void ScreenShotShortcut::checkAppInstallState()
{
    m_applicationInfo->desktopFilePathFromName(KYLIN_SCREENSHOT, m_appPath);
    if (m_isEnable == !m_appPath.isEmpty()) return;
    m_isEnable = !m_appPath.isEmpty();
    Q_EMIT enableStatusChanged(m_isEnable);
}

ScreenShotShortcutPlugin::ScreenShotShortcutPlugin(QObject *parent) : UkuiShortcutPlugin(parent)
{

}

QStringList ScreenShotShortcutPlugin::translations()
{
    return {QStringLiteral("screenshot-shortcut")};
}

Shortcut *ScreenShotShortcutPlugin::createShortcut()
{
    return new ScreenShotShortcut;
}
