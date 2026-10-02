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

#include "auto-rotation-shortcut.h"

#include <QDBusReply>
#include <QDebug>
#include <QApplication>

static const QString SERVICE = QStringLiteral("com.kylin.statusmanager.interface");
static const QString PATH = QStringLiteral("/");
static const QString INTERFACE = QStringLiteral("com.kylin.statusmanager.interface");
static const QString MODE_CHANGE_SIGNAL = QStringLiteral("mode_change_signal");
static const QString GET_TABLETMODE_METHOD = QStringLiteral("get_current_tabletmode");
static const QString GET_AUTO_ROTATION_METHOD = QStringLiteral("get_auto_rotation");
static const QString SET_AUTO_ROTATION_METHOD = QStringLiteral("set_auto_rotation");
static const QString AUTO_ROTATION_CHANGE_SIGNAL = QStringLiteral("auto_rotation_change_signal");
static const QString IS_SUPPORTED_AUTOROTATION = QStringLiteral("is_supported_autorotation");

static const QString AUTO_ROTATION_ICON = QStringLiteral("screen-rotation-landscape-24-symbolic");

using namespace UkuiShortcut;
AutoRotationShortcut::AutoRotationShortcut(QObject *parent) : Shortcut(parent)
{
    //set initial status info
    m_currentStatusInfo.setColor(Color::ColorRole::BaseColor);
    m_currentStatusInfo.setName(tr("Auto Rotation"));
    m_currentStatusInfo.setToolTip(tr("Auto Rotation"));
    m_currentStatusInfo.setIcon(AUTO_ROTATION_ICON);

    PluginMetaData pc {true, 1, PluginMetaType::PluginType::Icon};
    PluginMetaData tablet {true, 1, PluginMetaType::PluginType::Icon};
    m_metaData.insert(PluginMetaType::SystemMode::PC, pc);
    m_metaData.insert(PluginMetaType::SystemMode::Tablet, tablet);

    m_statusManagerIface = new QDBusInterface(SERVICE, PATH, INTERFACE, QDBusConnection::sessionBus(), this);
    if(!m_statusManagerIface->isValid()) {
        qWarning() << "StatusChangeShortcut error:" << m_statusManagerIface->lastError();
        m_isEnable = false;
        return;
    }

    QDBusReply<bool> rotationEnable = m_statusManagerIface->call(IS_SUPPORTED_AUTOROTATION);
    if (rotationEnable.isValid()) {
        if (rotationEnable.value()) {
            m_isEnable = false;
            return;
        }
    } else {
        qWarning() << "AutoRotationShortcut error, call" << IS_SUPPORTED_AUTOROTATION << "failed!";
    }

    QDBusReply<bool> rotationReply = m_statusManagerIface->call(GET_AUTO_ROTATION_METHOD);
    if(rotationReply.isValid()) {
        m_isEnable = true;
        m_autoRotationEnable = rotationReply.value();
        m_currentStatusInfo.setColor(rotationReply.value()? Color::ColorRole::HighLight: Color::ColorRole::BaseColor);
    } else {
        m_isEnable = false;
        qWarning() << "AutoRotationShortcut error, call" << GET_AUTO_ROTATION_METHOD << "failed!";
    }

    QDBusReply<bool> reply = m_statusManagerIface->call(GET_TABLETMODE_METHOD);
    if(reply.isValid()) {
        m_isEnable = reply.value();
    } else {
        qWarning() << "AutoRotationShortcut error, call" << GET_TABLETMODE_METHOD << "failed!";
    }

    if(!QDBusConnection::sessionBus().connect(SERVICE, PATH, INTERFACE, MODE_CHANGE_SIGNAL, this, SLOT(statusChangeSlot(bool)))) {
        qWarning() << "AutoRotationShortcut error, connect " << MODE_CHANGE_SIGNAL << "failed!";
    }

    if(!QDBusConnection::sessionBus().connect(SERVICE, PATH, INTERFACE, AUTO_ROTATION_CHANGE_SIGNAL, this, SLOT(autoRotationChangeSlot(bool)))) {
        qWarning() << "AutoRotationShortcut error, connect " << AUTO_ROTATION_CHANGE_SIGNAL << "failed!";
    }
}

AutoRotationShortcut::~AutoRotationShortcut()
{
}

void AutoRotationShortcut::active(PluginMetaType::Action action)
{
    if(PluginMetaType::Actions(PluginMetaType::Click) == action) {
        QDBusReply<bool> reply = m_statusManagerIface->call(SET_AUTO_ROTATION_METHOD, !m_autoRotationEnable, "ukui-sidebar", "setAutoRotation");
        if(!reply.isValid()) {
            qWarning() << "AutoRotationShortcut error, call" << SET_AUTO_ROTATION_METHOD << "failed!";
        }
    }
}

const StatusInfo UkuiShortcut::AutoRotationShortcut::currentStatus()
{
    return m_currentStatusInfo;
}

bool AutoRotationShortcut::isEnable()
{
    return m_isEnable;
}

void AutoRotationShortcut::statusChangeSlot(bool isTabletMode)
{
    if(m_isEnable != isTabletMode) {
        m_isEnable = isTabletMode;
        Q_EMIT enableStatusChanged(m_isEnable);
    }
}

void AutoRotationShortcut::autoRotationChangeSlot(bool autoRotation)
{
    m_autoRotationEnable = autoRotation;
    m_currentStatusInfo.setColor(autoRotation? Color::ColorRole::HighLight: Color::ColorRole::BaseColor);
    if(m_isEnable)
        Q_EMIT statusChanged(m_currentStatusInfo);
}

QMap<PluginMetaType::SystemMode, PluginMetaData> AutoRotationShortcut::pluginMetaData()
{
    return m_metaData;
}

// == plugin == //
QStringList AutoRotationShortcutPlugin::translations()
{
    return {QStringLiteral("auto-rotation-shortcut")};
}

Shortcut *AutoRotationShortcutPlugin::createShortcut()
{
    return new AutoRotationShortcut;
}

AutoRotationShortcutPlugin::AutoRotationShortcutPlugin(QObject *parent) : UkuiShortcutPlugin(parent)
{

}
