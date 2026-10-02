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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */
#include "status-change-shortcut.h"
#include <QDBusConnection>
#include <QDBusReply>
#include <QDebug>
#include <QDBusPendingCall>
#include <QDBusPendingReply>
#include <QApplication>
#include <QTranslator>
#include <kysdk/kysdk-system/libkysysinfo.h>

static const QString SERVICE = QStringLiteral("com.kylin.statusmanager.interface");
static const QString PATH = QStringLiteral("/");
static const QString INTERFACE = QStringLiteral("com.kylin.statusmanager.interface");
static const QString MODE_CHANGE_SIGNAL = QStringLiteral("mode_change_signal");
static const QString MODE_ENABLE_CHANGE_SIGNAL = QStringLiteral("isTabletModeEnabledChanged");
static const QString GET_TABLET_MODE_ENABLE = QStringLiteral("isTabletModeEnabled");
static const QString GET_TABLETMODE_METHOD = QStringLiteral("get_current_tabletmode");
static const QString SET_TABLETMODE_METHOD = QStringLiteral("set_tabletmode");
static const QString TABLETMODE_ICON = QStringLiteral("tablet-24-symbolic");

using namespace UkuiShortcut;
StatusChangeShortcut::StatusChangeShortcut()
{
    m_statusManagerIface = new QDBusInterface(SERVICE, PATH, INTERFACE, QDBusConnection::sessionBus(), this);
    if(!m_statusManagerIface->isValid()) {
        qWarning() << "StatusChangeShortcut error:" << m_statusManagerIface->lastError();
        m_isEnable = false;
        return;
    }

    auto translator = new QTranslator(this);
    try {
        if(!translator->load(QString(TRANSLATION_FILE_DIR) + "/status-change-shortcut_" + QLocale::system().name() + ".qm")) throw -1;
        QApplication::installTranslator(translator);
    } catch(...) {
        qWarning() << "StatusChangeShortcut load translations file" << QLocale::system().name() << "failed!";
    }

    initMetaData();
    initInfo();

    //We can't guarantee the stability of "kylin-status-manager", sou do it cautiously.
    QDBusPendingCall asyncCall = m_statusManagerIface->asyncCall(GET_TABLET_MODE_ENABLE);
    connect(new QDBusPendingCallWatcher(asyncCall, this), &QDBusPendingCallWatcher::finished, [ & ](QDBusPendingCallWatcher *call) {
        if (call) {
            QDBusPendingReply<bool> reply = *call;
            enableChanged(!reply.isError() && reply.value());
            call->deleteLater();
        }
    });

    if(!QDBusConnection::sessionBus().connect(SERVICE, PATH, INTERFACE, MODE_CHANGE_SIGNAL, this, SLOT(statusChangeSlot(bool)))) {
        qWarning() << "StatusChangeShortcut error, connect " << MODE_CHANGE_SIGNAL << "failed!";
    }

    if(!QDBusConnection::sessionBus().connect(SERVICE, PATH, INTERFACE, MODE_ENABLE_CHANGE_SIGNAL, this, SLOT(enableChanged(bool)))) {
        qWarning() << "StatusChangeShortcut error, connect " << MODE_ENABLE_CHANGE_SIGNAL << "failed!";
    }
}

void StatusChangeShortcut::initInfo()
{
    //set initial status info
    m_currentStatusInfo.setIcon(TABLETMODE_ICON);

    char* systemCategory = kdk_system_get_systemCategory();
    QString category(systemCategory);
    if (category == "MaxTablet") {
        m_currentStatusInfo.setName(tr("Max Tablet"));
    } else {
        m_currentStatusInfo.setName(tr("Tablet Mode"));
    }

    m_currentStatusInfo.setColor(Color::ColorRole::BaseColor);

    free(systemCategory);
}

StatusChangeShortcut::~StatusChangeShortcut()
{


}

QString StatusChangeShortcut::pluginId()
{
    return QStringLiteral("StatusChangeShortcut");
}

void StatusChangeShortcut::active(PluginMetaType::Action action)
{
    if(PluginMetaType::Action::Click == action) {
        m_statusManagerIface->asyncCall(SET_TABLETMODE_METHOD, !m_isTabletMode, "ukui-sidebar", "changemode");
    }
}

const StatusInfo StatusChangeShortcut::currentStatus()
{
    return m_currentStatusInfo;
}

bool StatusChangeShortcut::isEnable()
{
    return m_isEnable;
}

void StatusChangeShortcut::statusChangeSlot(bool isTabletMode)
{
    if(m_isTabletMode == isTabletMode)
        return;
    setStatusInfo(isTabletMode);
    Q_EMIT statusChanged(m_currentStatusInfo);          
}

void StatusChangeShortcut::setStatusInfo(bool isTabletMode)
{
    qDebug() << "StatusChangeShortcut::setStatusInfo" << isTabletMode;
    m_isTabletMode = isTabletMode;
    m_currentStatusInfo.setColor(isTabletMode ? Color::ColorRole::HighLight : Color::ColorRole::BaseColor);
}

QMap<PluginMetaType::SystemMode, PluginMetaData> StatusChangeShortcut::pluginMetaData()
{
    return m_metaData;
}

void StatusChangeShortcut::initMetaData()
{
    PluginMetaData pc {true, 0, PluginMetaType::PluginType::Icon};
    PluginMetaData tablet {true, 0, PluginMetaType::PluginType::Icon};

    pc.setPreAction(PluginMetaType::Hide);
    tablet.setPreAction(PluginMetaType::Hide);

    m_metaData.insert(PluginMetaType::SystemMode::PC, pc);
    m_metaData.insert(PluginMetaType::SystemMode::Tablet, tablet);
}

void StatusChangeShortcut::enableChanged(bool isEnable)
{
    if (m_isEnable == isEnable) {
        return;
    }

    m_isEnable = isEnable;
    if (m_isEnable) {
        // 功能可用后，主动获取当前状态
        QDBusPendingCall asyncCall = m_statusManagerIface->asyncCall(GET_TABLETMODE_METHOD);
        connect(new QDBusPendingCallWatcher(asyncCall, this), &QDBusPendingCallWatcher::finished, [ & ](QDBusPendingCallWatcher *call) {
            if (call) {
                QDBusPendingReply<bool> reply = *call;
                if (reply.isError()) {
                    enableChanged(false);
                } else {
                    statusChangeSlot(reply.value());
                }
                call->deleteLater();
            }
        });
    }

    Q_EMIT enableStatusChanged(m_isEnable);
}

StatusChangeShortcutPlugin::StatusChangeShortcutPlugin(QObject *parent) : UkuiShortcutPlugin(parent)
{

}

QStringList StatusChangeShortcutPlugin::translations()
{
    return {QStringLiteral("status-change-shortcut")};
}

Shortcut *StatusChangeShortcutPlugin::createShortcut()
{
    return new StatusChangeShortcut;
}
