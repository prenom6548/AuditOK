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

#include "services-and-supports-shortcut.h"
#include <QDBusInterface>
#include <QDBusReply>
#include <QProcess>
#include <QFile>
#include <QIcon>

#define KYLIN_SERVICE_SUPPORT               "/usr/share/applications/kylin-os-manager.desktop"
#define THEME_LOTTIE_PATH                   "/usr/share/icons/ukui-motion-icons"

using namespace UkuiShortcut;

ServicesAndSupportsShortcutPlugin::ServicesAndSupportsShortcutPlugin(QObject *parent)
{

}

QStringList ServicesAndSupportsShortcutPlugin::translations()
{
    return {QStringLiteral("services-and-supports-shortcut")};
}


Shortcut *ServicesAndSupportsShortcutPlugin::createShortcut()
{
    return new ServicesAndSupportsShortcut;
}

ServicesAndSupportsShortcut::ServicesAndSupportsShortcut(QObject *parent)
{
    m_currentStatus.setName(tr("Services and Supports"));
    m_currentStatus.setIcon(QFile::exists(THEME_LOTTIE_PATH "/" UKUI_SERVICE_SUPPORTS_LOTTIE)
                            ? UKUI_SERVICE_SUPPORTS_LOTTIE
                            : (QIcon::fromTheme(UKUI_SERVICE_SUPPORTS_SYMBOLIC).isNull()
                               ? "://ukui-service-support-symbolic.svg" : UKUI_SERVICE_SUPPORTS_SYMBOLIC));
    m_currentStatus.setToolTip(tr("Services and Supports"));
    m_currentStatus.setColor(Color::ColorRole::BaseColor);
    initMetaData();
    m_isEnable = QFile::exists(KYLIN_SERVICE_SUPPORT);

    m_applicationInfo = new UkuiSearch::ApplicationInfo();
    connect(m_applicationInfo, &UkuiSearch::ApplicationInfo::appDBItems2BDelete, this, &ServicesAndSupportsShortcut::onAppUninstalled);
    connect(m_applicationInfo, &UkuiSearch::ApplicationInfo::appDBItems2BAdd, this, &ServicesAndSupportsShortcut::onAppInstalled);
}

ServicesAndSupportsShortcut::~ServicesAndSupportsShortcut()
{
    if (m_applicationInfo) {
        delete  m_applicationInfo;
        m_applicationInfo = nullptr;
    }
}

QMap<PluginMetaType::SystemMode, PluginMetaData> ServicesAndSupportsShortcut::pluginMetaData()
{
    return m_metaData;
}

void ServicesAndSupportsShortcut::active(PluginMetaType::Action action)
{
    switch (action) {
        default:
        case PluginMetaType::Action::Click:
            openServicesAndSupports();
            break;
        case PluginMetaType::Action::LongClick:
        case PluginMetaType::Action::MenuRequest:
        break;
    }
}

const StatusInfo ServicesAndSupportsShortcut::currentStatus()
{
    return m_currentStatus;
}

bool ServicesAndSupportsShortcut::isEnable()
{
    return m_isEnable;
}

void ServicesAndSupportsShortcut::setAddedState(bool added)
{
    if (m_isAdded != added)
        m_isAdded = added;
}

void ServicesAndSupportsShortcut::initMetaData()
{
    PluginMetaData metadata {true, 13, PluginMetaType::PluginType::Icon};
    metadata.setPreAction(PluginMetaType::Hide);

    m_metaData.insert(PluginMetaType::SystemMode::PC, metadata);
    m_metaData.insert(PluginMetaType::SystemMode::Tablet, metadata);
}

void ServicesAndSupportsShortcut::openServicesAndSupports()
{
    if (!m_isEnable || !m_isAdded)
        return;

    QDBusMessage message = QDBusMessage::createMethodCall(KYLIN_APP_MANAGER_NAME,
                                                          KYLIN_APP_MANAGER_PATH,
                                                          KYLIN_APP_MANAGER_INTERFACE,
                                                          "LaunchAppWithArguments");
    QStringList arg("-jumpTab=ServiceSupport");
    message << KYLIN_SERVICE_SUPPORT << arg;

    auto watcher = new QDBusPendingCallWatcher(QDBusConnection::sessionBus().asyncCall(message), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [] (QDBusPendingCallWatcher *self) {
        if (self) {
            if (self->isError()) {
                qDebug() << "ServicesAndSupportsShortcut : Using dbus failed, " << self->error().message();
                QProcess::startDetached("/usr/bin/kylin-os-manager -jumpTab=ServiceSupport");
            }
            self->deleteLater();
        }
    });
}

void ServicesAndSupportsShortcut::onAppUninstalled(const QStringList &desktopFiles)
{
    if (desktopFiles.contains(KYLIN_SERVICE_SUPPORT)) {
        m_isEnable = false;
        Q_EMIT enableStatusChanged(m_isEnable);
    }
}

void ServicesAndSupportsShortcut::onAppInstalled(const QStringList &desktopFiles)
{
    if (desktopFiles.contains(KYLIN_SERVICE_SUPPORT)) {
        m_isEnable = true;
        Q_EMIT enableStatusChanged(m_isEnable);
    }
}
