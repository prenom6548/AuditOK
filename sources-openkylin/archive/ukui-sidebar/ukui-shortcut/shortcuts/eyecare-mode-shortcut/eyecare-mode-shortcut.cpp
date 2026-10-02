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

#include "eyecare-mode-shortcut.h"
#include <QDebug>
#include <QIcon>
#include <dbus-connector.h>
#include <QFile>

#define SETTINGS_DAEMON_PLUGIN_COLOR          "org.ukui.SettingsDaemon.plugins.color"
#define EYECARE_KEY         "mainSwitch"
#define UKUI_EYECARE_SYMBOLIC                 "eye-open-negative-filled-24-symbolic"
#define UKUI_EYECARE_LOTTIE                   "eye-open-negative-filled-24-symbolic-motion.lottie"
#define THEME_LOTTIE_PATH                     "/usr/share/icons/ukui-motion-icons"

using namespace UkuiShortcut;
using namespace UkuiQuick;
UkuiShortcut::EyeCareModeShortcut::EyeCareModeShortcut(QObject *parent) : Shortcut(parent)
{
    m_currentStatus.setName(tr("Eyecare Mode"));
    m_currentStatus.setIcon(QFile::exists(THEME_LOTTIE_PATH "/" UKUI_EYECARE_LOTTIE)
                            ? UKUI_EYECARE_LOTTIE
                            : (QIcon::fromTheme(UKUI_EYECARE_SYMBOLIC).isNull()
                               ? ":/eyecareModeShortcut/ukui-eyecare.svg" : UKUI_EYECARE_SYMBOLIC));
    m_currentStatus.setToolTip(tr("Eyecare Mode"));
    initMetaData();

    DbusConnector* dbusConnector = new DbusConnector("org.ukui.SettingsDaemon", "/GlobalSignal",
                                                     "org.ukui.SettingsDaemon.GlobalSignal",
                                                     QDBusConnection::sessionBus(), this);
    connect(dbusConnector, &DbusConnector::ready, this, &EyeCareModeShortcut::onDbusConnectorReady);
    dbusConnector->start();
}

UkuiShortcut::EyeCareModeShortcut::~EyeCareModeShortcut()
{
    if (m_gsettings) {
        delete m_gsettings;
        m_gsettings = nullptr;
    }
}

void EyeCareModeShortcut::onQGSettingsChanged(const QString &key)
{
    if (key == EYECARE_KEY) {
        setIconButtonColor();
    }
}

void EyeCareModeShortcut::onDbusConnectorReady(QDBusInterface *interface)
{
    if (!interface || !interface->isValid()) return;
    auto watcher = new QDBusPendingCallWatcher(interface->asyncCall(QStringLiteral("isSupportGamma")));
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this] (QDBusPendingCallWatcher *self) {
        if (self) {
            if (self->isError()) {
                qDebug() << "eyecare-mode-shortcut: asyncCall isSupportGamma failed !!!" << self->error().message();
            } else {
                QDBusPendingReply<bool> reply = *self;
                if (reply.value()) {
                    const QByteArray id(SETTINGS_DAEMON_PLUGIN_COLOR);
                    if (QGSettings::isSchemaInstalled(id)) {
                        m_gsettings = new QGSettings(id);
                        if (!m_gsettings->keys().contains(EYECARE_KEY, Qt::CaseInsensitive)) {
                            qDebug() << "eyecare-mode-shortcut:" << "can't find key :" << EYECARE_KEY;
                        } else {
                            setEnable(true);
                            if (m_isAdded) {
                                connect(m_gsettings, &QGSettings::changed, this,&EyeCareModeShortcut::onQGSettingsChanged);
                            }
                            setIconButtonColor();
                        }
                    } else {
                        qDebug() << "eyecare-mode-shortcut:" << "can't find gsettings :" << SETTINGS_DAEMON_PLUGIN_COLOR;
                    }
                } else {
                    qDebug() << "eyecare-mode-shortcut: does not support eyecare mode !!!";
                }
            }
            self->deleteLater();
        }
    });
}

void EyeCareModeShortcut::setIconButtonColor()
{
    if(m_gsettings->get(EYECARE_KEY).toBool() && m_isAdded) {
        m_currentStatus.setColor(Color::ColorRole::HighLight);
    } else {
        m_currentStatus.setColor(Color::ColorRole::BaseColor);
    }
    Q_EMIT statusChanged(m_currentStatus);
}

bool EyeCareModeShortcut::isEnable()
{
    return m_isEnable;
}

void EyeCareModeShortcut::setAddedState(bool added)
{
    if (m_isAdded != added) {
        m_isAdded = added;
        if (!m_gsettings) return;
        if (m_isAdded) {
            connect(m_gsettings, &QGSettings::changed, this,&EyeCareModeShortcut::onQGSettingsChanged);
        } else {
            disconnect(m_gsettings, &QGSettings::changed, this,&EyeCareModeShortcut::onQGSettingsChanged);
        }
        setIconButtonColor();
    }
}

void EyeCareModeShortcut::setEnable(bool isEnable)
{
    if (m_isEnable == isEnable) return;
    m_isEnable = isEnable;
    Q_EMIT enableStatusChanged(m_isEnable);
}

void EyeCareModeShortcut::active(PluginMetaType::Action action)
{
    if (action == PluginMetaType::Action::Click) {
        trigger();
    }
}

const StatusInfo EyeCareModeShortcut::currentStatus()
{
    return m_currentStatus;
}

QMap<PluginMetaType::SystemMode, PluginMetaData> EyeCareModeShortcut::pluginMetaData()
{
    return m_metaData;
}

void EyeCareModeShortcut::initMetaData()
{
    PluginMetaData pc {true, 9, PluginMetaType::PluginType::Icon};
    PluginMetaData tablet {true, 9, PluginMetaType::PluginType::Icon};
    m_metaData.insert(PluginMetaType::SystemMode::PC, pc);
    m_metaData.insert(PluginMetaType::SystemMode::Tablet, tablet);
}

void EyeCareModeShortcut::trigger()
{
    if (!m_isEnable || !m_isAdded) return;
    if (m_gsettings) {
        if (m_gsettings->keys().contains(EYECARE_KEY, Qt::CaseInsensitive)) {
            m_gsettings->set(EYECARE_KEY, !m_gsettings->get(EYECARE_KEY).toBool());
        }
    }
}

EyeCareModeShortcutPlugin::EyeCareModeShortcutPlugin(QObject *parent) : UkuiShortcutPlugin(parent)
{

}

QStringList EyeCareModeShortcutPlugin::translations()
{
    return {QStringLiteral("eyecare-mode-shortcut")};
}

Shortcut *EyeCareModeShortcutPlugin::createShortcut()
{
    return new EyeCareModeShortcut;
}
