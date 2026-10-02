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

#include "connectivity-shortcut.h"
#include <QDBusInterface>
#include <QDBusReply>
#include <QProcess>
#include <QFile>
#include <QIcon>

#define KYLIN_CONNECTIVITY             "/usr/share/applications/kylin-connectivity.desktop"
#define UKUI_PHONE_DOCKED_SYMBOLIC     "phone-docked-24-symbolic"
#define UKUI_PHONE_DOCKED_LOTTIE       "phone-docked-24-symbolic-motion.lottie"
#define THEME_LOTTIE_PATH              "/usr/share/icons/ukui-motion-icons"

using namespace UkuiShortcut;

ConnectivityShortcutPlugin::ConnectivityShortcutPlugin(QObject *parent)
{

}

QStringList ConnectivityShortcutPlugin::translations()
{
    return {QStringLiteral("connectivity-shortcut")};
}


Shortcut *ConnectivityShortcutPlugin::createShortcut()
{
    return new ConnectivityShortcut;
}

ConnectivityShortcut::ConnectivityShortcut(QObject *parent)
{
    m_currentStatus.setName(tr("Connectivity"));
    m_currentStatus.setIcon(QFile::exists(THEME_LOTTIE_PATH "/" UKUI_PHONE_DOCKED_LOTTIE)
                            ? UKUI_PHONE_DOCKED_LOTTIE
                            : (QIcon::fromTheme(UKUI_PHONE_DOCKED_SYMBOLIC).isNull()
                               ? "://icon/ukui-connectivity-symbolic.svg" : UKUI_PHONE_DOCKED_SYMBOLIC));
    m_currentStatus.setToolTip(tr("Connectivity"));
    m_currentStatus.setColor(Color::ColorRole::BaseColor);
    initMetaData();
    m_isEnable = QFile::exists(KYLIN_CONNECTIVITY);

    m_applicationInfo = new UkuiSearch::ApplicationInfo();
    connect(m_applicationInfo, &UkuiSearch::ApplicationInfo::appDBItems2BDelete, this, &ConnectivityShortcut::onAppUninstalled);
    connect(m_applicationInfo, &UkuiSearch::ApplicationInfo::appDBItems2BAdd, this, &ConnectivityShortcut::onAppInstalled);
}

ConnectivityShortcut::~ConnectivityShortcut()
{
    if (m_applicationInfo) {
        delete  m_applicationInfo;
        m_applicationInfo = nullptr;
    }
}

QMap<PluginMetaType::SystemMode, PluginMetaData> ConnectivityShortcut::pluginMetaData()
{
    return m_metaData;
}

void ConnectivityShortcut::active(PluginMetaType::Action action)
{
    switch (action) {
        default:
        case PluginMetaType::Action::Click:
            openConnectivity();
            break;
        case PluginMetaType::Action::LongClick:
        case PluginMetaType::Action::MenuRequest:
        break;
    }
}

const StatusInfo ConnectivityShortcut::currentStatus()
{
    return m_currentStatus;
}

bool ConnectivityShortcut::isEnable()
{
    return m_isEnable;
}

void ConnectivityShortcut::setAddedState(bool added)
{
    if (m_isAdded != added)
        m_isAdded = added;
}

void ConnectivityShortcut::initMetaData()
{
    PluginMetaData metadata {true, 14, PluginMetaType::PluginType::Icon};
    metadata.setPreAction(PluginMetaType::Hide);

    m_metaData.insert(PluginMetaType::SystemMode::PC, metadata);
    m_metaData.insert(PluginMetaType::SystemMode::Tablet, metadata);
}

void ConnectivityShortcut::openConnectivity()
{
    if (!m_isEnable || !m_isAdded)
        return;

    QDBusMessage message = QDBusMessage::createMethodCall(KYLIN_APP_MANAGER_NAME,
                                                          KYLIN_APP_MANAGER_PATH,
                                                          KYLIN_APP_MANAGER_INTERFACE,
                                                          "LaunchApp");
    message << KYLIN_CONNECTIVITY;

    auto watcher = new QDBusPendingCallWatcher(QDBusConnection::sessionBus().asyncCall(message), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [] (QDBusPendingCallWatcher *self) {
        if (self) {
            if (self->isError()) {
                qDebug() << "ConnectivityShortcut : Using dbus failed, " << self->error().message();
                QProcess::startDetached("kylin-connectivity");
            }
            self->deleteLater();
        }
    });
}

void ConnectivityShortcut::onAppUninstalled(const QStringList &desktopFiles)
{
    if (desktopFiles.contains(KYLIN_CONNECTIVITY)) {
        m_isEnable = false;
        Q_EMIT enableStatusChanged(m_isEnable);
    }
}

void ConnectivityShortcut::onAppInstalled(const QStringList &desktopFiles)
{
    if (desktopFiles.contains(KYLIN_CONNECTIVITY)) {
        m_isEnable = true;
        Q_EMIT enableStatusChanged(m_isEnable);
    }
}
