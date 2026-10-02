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

#include "vpn-shortcut.h"
#include <QDebug>
#include <QColor>
#include <QIcon>
#include <QFile>

#define KYLIN_VPN_MANAGER_NAME         "com.kylin.kylinvpn"
#define KYLIN_VPN_MANAGER_PATH         "/com/kylin/kylinvpn"
#define KYLIN_VPN_MANAGER_INTERFACE    "com.kylin.kylinvpn"
#define UKUI_VPN_SYMBOLIC              "network-vpn-24-symbolic"
#define UKUI_VPN_LOTTIE                "network-vpn-24-symbolic-motion.lottie"
#define THEME_LOTTIE_PATH              "/usr/share/icons/ukui-motion-icons"

using namespace UkuiShortcut;
VPNShortcut::VPNShortcut(QObject *parent) : Shortcut(parent)
{
    m_currentStatusInfo.setColor(Color::ColorRole::BaseColor);
    m_currentStatusInfo.setName(tr("VPN"));
    m_currentStatusInfo.setIcon(QFile::exists(THEME_LOTTIE_PATH "/" UKUI_VPN_LOTTIE)
                                ? UKUI_VPN_LOTTIE
                                : (QIcon::fromTheme(UKUI_VPN_SYMBOLIC).isNull()
                                   ? ":/vpnShortcut/ukui-vpn.svg" : UKUI_VPN_SYMBOLIC));
    m_currentStatusInfo.setToolTip(tr("VPN"));
    m_isEnable = initVpnManagerDbusInterface();
    initVpnServiceWatcher();
    initMetaData();
}

VPNShortcut::~VPNShortcut()
{
    if (!m_vpnManagerDbusInterface) {
        delete m_vpnManagerDbusInterface;
        m_vpnManagerDbusInterface = nullptr;
    }
    if (!m_vpnServiceWatcher) {
        delete m_vpnServiceWatcher;
        m_vpnServiceWatcher = nullptr;
    }
}


bool VPNShortcut::initVpnManagerDbusInterface()
{
    m_vpnManagerDbusInterface = new QDBusInterface(KYLIN_VPN_MANAGER_NAME,
                                                   KYLIN_VPN_MANAGER_PATH,
                                                   KYLIN_VPN_MANAGER_INTERFACE,
                                                   QDBusConnection::sessionBus());
    if (!m_vpnManagerDbusInterface->isValid()) {
        qDebug()<<"vpn_shortcut: m_vpnManagerDbusInterface init error";
        delete m_vpnManagerDbusInterface;
        m_vpnManagerDbusInterface = nullptr;
        return false;
    }
    return true;
}

void VPNShortcut::initVpnServiceWatcher()
{
    m_vpnServiceWatcher = new QDBusServiceWatcher(KYLIN_VPN_MANAGER_NAME,
                                                  QDBusConnection::sessionBus(),
                                                  QDBusServiceWatcher::WatchForRegistration |
                                                  QDBusServiceWatcher::WatchForUnregistration,this);
    if (m_vpnServiceWatcher) {
        connect(m_vpnServiceWatcher, &QDBusServiceWatcher::serviceRegistered, this, &VPNShortcut::onServiceRegistered);
        connect(m_vpnServiceWatcher, &QDBusServiceWatcher::serviceUnregistered, this, &VPNShortcut::onServiceUnregistered);
    } else {
        qDebug()<<"vpn_shortcut: m_vpnServiceWatcher init error";
    }
}

void VPNShortcut::onServiceRegistered()
{
    if (!m_vpnManagerDbusInterface) {
        m_isEnable = initVpnManagerDbusInterface();
        Q_EMIT enableStatusChanged(m_isEnable);
    }
}

void VPNShortcut::onServiceUnregistered()
{
    if (m_vpnManagerDbusInterface) {
        delete m_vpnManagerDbusInterface;
        m_vpnManagerDbusInterface = nullptr;
    }
    m_isEnable = false;
    Q_EMIT enableStatusChanged(m_isEnable);
}

QMap<PluginMetaType::SystemMode, PluginMetaData> VPNShortcut::pluginMetaData()
{
    return m_metaData;
}

QString VPNShortcut::pluginId()
{
    return QStringLiteral("VPN");
}

void VPNShortcut::active(PluginMetaType::Action action)
{
    if (action == PluginMetaType::Action::Click) {
        trigger();
    }
}

const StatusInfo VPNShortcut::currentStatus()
{
    return m_currentStatusInfo;
}

bool VPNShortcut::isEnable()
{
    return m_isEnable;
}

void VPNShortcut::setAddedState(bool added)
{
    if (m_isAdded != added) {
        m_isAdded = added;
    }
}

void VPNShortcut::trigger()
{
    if (!m_isEnable || !m_isAdded) return;
    auto watcher = new QDBusPendingCallWatcher(m_vpnManagerDbusInterface->asyncCall(QStringLiteral("showKylinVpn")),this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [ = ] (QDBusPendingCallWatcher *self) {
        if (self) {
            if (self->isError()) {
                qDebug()<<"vpn-shortcut: vpn is failed";
            }
            self->deleteLater();
        }
    });
}

void VPNShortcut::initMetaData()
{
    PluginMetaData pc {true, 12, PluginMetaType::PluginType::Icon};
    pc.setPreAction(PluginMetaType::Hide);
    PluginMetaData tablet {true, 12, PluginMetaType::PluginType::Icon};
    tablet.setPreAction(PluginMetaType::Hide);
    m_metaData.insert(PluginMetaType::SystemMode::PC, pc);
    m_metaData.insert(PluginMetaType::SystemMode::Tablet, tablet);
}

VPNShortcutPlugin::VPNShortcutPlugin(QObject *parent) : UkuiShortcutPlugin(parent)
{

}

QStringList VPNShortcutPlugin::translations()
{
    return {QStringLiteral("vpn-shortcut")};
}

Shortcut *VPNShortcutPlugin::createShortcut()
{
    return new VPNShortcut;
}
