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
 * Authors: Nicole <buxiaoqing@kylinos.cn>
 *
 */

#include "clipboard-shortcut.h"
#include <QIcon>
#include <QFile>
#include <QProcess>
#include <QDBusInterface>
#include <QDBusReply>

#define CLIPBOARD_DESKTOP              "/etc/xdg/autostart/ukui-clipboard-tray.desktop"
#define KYLIN_APP_MANAGER_NAME         "com.kylin.ProcessManager"
#define KYLIN_APP_MANAGER_PATH         "/com/kylin/ProcessManager/AppLauncher"
#define KYLIN_APP_MANAGER_INTERFACE    "com.kylin.ProcessManager.AppLauncher"
#define UKUI_CLIPBOARD_SYMBOLIC        "ukui-clipboard-24-symbolic"
#define UKUI_CLIPBOARD_LOTTIE          "ukui-clipboard-24-symbolic-motion.lottie"
#define THEME_LOTTIE_PATH              "/usr/share/icons/ukui-motion-icons"

using namespace UkuiShortcut;

ClipboardShortcutPlugin::ClipboardShortcutPlugin(QObject *parent)
{

}

QStringList ClipboardShortcutPlugin::translations()
{
    return {QStringLiteral("clipboard-shortcut")};
}


Shortcut *ClipboardShortcutPlugin::createShortcut()
{
    return new ClipboardShortcut;
}

ClipboardShortcut::ClipboardShortcut(QObject *parent)
{
    m_currentStatus.setName(tr("Clipboard"));
    m_currentStatus.setIcon(QFile::exists(THEME_LOTTIE_PATH "/" UKUI_CLIPBOARD_LOTTIE)
                            ? UKUI_CLIPBOARD_LOTTIE
                            : (QIcon::fromTheme(UKUI_CLIPBOARD_SYMBOLIC).isNull()
                               ? "://ukui-clipboard-symbolic.svg" : UKUI_CLIPBOARD_SYMBOLIC));
    m_currentStatus.setToolTip(tr("Clipboard"));
    m_currentStatus.setColor(Color::ColorRole::BaseColor);
    initMetaData();
    m_isEnable = QFile::exists(CLIPBOARD_DESKTOP);

    m_applicationInfo = new UkuiSearch::ApplicationInfo();
    connect(m_applicationInfo, &UkuiSearch::ApplicationInfo::appDBItems2BDelete, this, &ClipboardShortcut::onAppUninstalled);
    connect(m_applicationInfo, &UkuiSearch::ApplicationInfo::appDBItems2BAdd, this, &ClipboardShortcut::onAppInstalled);
}

ClipboardShortcut::~ClipboardShortcut()
{
    if (m_applicationInfo) {
        delete m_applicationInfo;
        m_applicationInfo = nullptr;
    }
}

QMap<PluginMetaType::SystemMode, PluginMetaData> ClipboardShortcut::pluginMetaData()
{
    return m_metaData;
}

void ClipboardShortcut::active(PluginMetaType::Action action)
{
    switch (action) {
        default:
        case PluginMetaType::Action::Click:
            openClipboard();
            break;
        case PluginMetaType::Action::LongClick:
        case PluginMetaType::Action::MenuRequest:
        break;
    }
}

const StatusInfo ClipboardShortcut::currentStatus()
{
    return m_currentStatus;
}

bool ClipboardShortcut::isEnable()
{
    return m_isEnable;
}

void ClipboardShortcut::setAddedState(bool added)
{
    if (m_isAdded != added)
        m_isAdded = added;
}

void ClipboardShortcut::initMetaData()
{
    PluginMetaData metadata {true, 6, PluginMetaType::PluginType::Icon};
    metadata.setPreAction(PluginMetaType::Hide);

    m_metaData.insert(PluginMetaType::SystemMode::PC, metadata);
    m_metaData.insert(PluginMetaType::SystemMode::Tablet, metadata);
}

void ClipboardShortcut::openClipboard()
{
    if (!m_isEnable || !m_isAdded)
        return;

    QDBusMessage message = QDBusMessage::createMethodCall(KYLIN_APP_MANAGER_NAME,
                                                          KYLIN_APP_MANAGER_PATH,
                                                          KYLIN_APP_MANAGER_INTERFACE,
                                                          "RunCommand");
    message << "ukui-clipboard-tray --showMainWindow";

    auto watcher = new QDBusPendingCallWatcher(QDBusConnection::sessionBus().asyncCall(message), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [] (QDBusPendingCallWatcher *self) {
        if (self) {
            if (self->isError()) {
                qDebug() << "ClipboardShortcut : Using dbus failed, " << self->error().message();
                QProcess::startDetached("ukui-clipboard-tray --showMainWindow");
            }
            self->deleteLater();
        }
    });
}

void ClipboardShortcut::onAppUninstalled(const QStringList &desktopFiles)
{
    if (desktopFiles.contains(CLIPBOARD_DESKTOP)) {
        m_isEnable = false;
        Q_EMIT enableStatusChanged(m_isEnable);
    }
}

void ClipboardShortcut::onAppInstalled(const QStringList &desktopFiles)
{
    if (desktopFiles.contains(CLIPBOARD_DESKTOP)) {
        m_isEnable = true;
        Q_EMIT enableStatusChanged(m_isEnable);
    }
}
