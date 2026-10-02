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
#include "brightness-shortcut.h"
#include "dbus-connector.h"

#include <QDebug>
#include <QTranslator>
#include <QApplication>
#include <QDBusReply>
#include <QDBusInterface>
#include <QtMath>

#define UKUI_SETTINGS_DAEMON_NAME                    "org.ukui.SettingsDaemon"
#define UKUI_GLOBAL_BRIGHTNESS_PATH                  "/GlobalBrightness"
#define UKUI_SETTINGS_DAEMON_BRIGHTNESS_INTERFACE    "org.ukui.SettingsDaemon.Brightness"

using namespace UkuiShortcut;
using namespace UkuiQuick;

BrightnessShortcut::BrightnessShortcut(QObject *parent) : Shortcut(parent)
{
    m_currentStatusInfo.setValue(0);
    m_currentStatusInfo.setIcon("ukui-light-0-24-symbolic");
    m_currentStatusInfo.setToolTip(tr("Brightness"));
    initMetaData();

    DbusConnector* dbusConnector = new DbusConnector(UKUI_SETTINGS_DAEMON_NAME,
                                                     UKUI_GLOBAL_BRIGHTNESS_PATH,
                                                     UKUI_SETTINGS_DAEMON_BRIGHTNESS_INTERFACE,
                                                     QDBusConnection::sessionBus());
    connect(dbusConnector, &DbusConnector::ready, this, &BrightnessShortcut::onDbusConnectorReady);
    dbusConnector->start();
}

BrightnessShortcut::~BrightnessShortcut() = default;

QString BrightnessShortcut::pluginId()
{
    return QStringLiteral("BrightnessShortcut");
}

void BrightnessShortcut::active(PluginMetaType::Action action)
{
    Q_UNUSED(action);
}

void BrightnessShortcut::setValue(int value)
{
    if (m_isEnable) {
        if (m_interface && m_interface->isValid()) {
            if (m_currentBrightness == value) return;
            m_interface->asyncCall("setPrimaryBrightness", static_cast<quint32>(value));
        }
    }
}

const StatusInfo BrightnessShortcut::currentStatus()
{
    return m_currentStatusInfo;
}

bool BrightnessShortcut::isEnable()
{
    return m_isEnable;
}

void BrightnessShortcut::changeValue(quint32 brightness)
{
    if(m_currentBrightness == brightness) {
        return;
    }
    m_currentBrightness = brightness;
    updateStatusInfo(qFloor(brightness));
}

void BrightnessShortcut::updateStatusInfo(quint32 brightness)
{
    if(brightness >= 0 && brightness <= m_enhancedBrightness) {
        if(brightness == 0) {
            m_currentStatusInfo.setIcon("ukui-light-0-24-symbolic");
        } else if (brightness <= 25) {
            m_currentStatusInfo.setIcon("ukui-light-25-24-symbolic");
        } else if (brightness <= 50) {
            m_currentStatusInfo.setIcon("ukui-light-50-24-symbolic");
        } else if (brightness <= 75) {
            m_currentStatusInfo.setIcon("ukui-light-75-24-symbolic");
        } else {
            m_currentStatusInfo.setIcon("ukui-light-100-24-symbolic");
        }
        m_currentStatusInfo.setValue(static_cast<int>(brightness));

    } else {
        qWarning() << "Brightness value error: " << brightness;
        m_currentStatusInfo.setDisable(true);
        m_currentStatusInfo.setValue(0);
        m_currentStatusInfo.setIcon("ukui-light-0-24-symbolic");
    }

    Q_EMIT statusChanged(m_currentStatusInfo);
}

void BrightnessShortcut::setMaxValue()
{
    m_enhancedBrightness = m_enhance ? 150 : 100;
    m_currentStatusInfo.setMaxValue(m_enhancedBrightness);
    Q_EMIT statusChanged(m_currentStatusInfo);
}

QMap<PluginMetaType::SystemMode, PluginMetaData> BrightnessShortcut::pluginMetaData()
{
    return m_metaData;
}

void BrightnessShortcut::initMetaData()
{
    PluginMetaData pc {true, 0, PluginMetaType::PluginType::ProgressBar};
    PluginMetaData tablet {true, 0, PluginMetaType::PluginType::ProgressBar};
    m_metaData.insert(PluginMetaType::SystemMode::PC, pc);
    m_metaData.insert(PluginMetaType::SystemMode::Tablet, tablet);
}

void BrightnessShortcut::onDbusConnectorReady(QDBusInterface *interface)
{
    if (!interface || !interface->isValid()) return;
    m_interface = interface;

    QDBusReply<bool> isEnable = m_interface->call("isEnable");
    if (isEnable.isValid()) {
        if (m_isEnable != isEnable.value()) {
            m_isEnable = isEnable.value();
            Q_EMIT enableStatusChanged(m_isEnable);
        }
    }

    QDBusReply<bool> enhance = m_interface->call("isEnhancementBrightness");
    if (enhance.isValid()) {
        m_enhance = enhance.value();
    }
    setMaxValue();

    QDBusReply<quint32> v = m_interface->call("getPrimaryBrightness");
    changeValue(v.isValid() ? v.value() : 0);

    if (!QDBusConnection::sessionBus().connect(UKUI_SETTINGS_DAEMON_NAME,
                                               UKUI_GLOBAL_BRIGHTNESS_PATH,
                                               UKUI_SETTINGS_DAEMON_BRIGHTNESS_INTERFACE, "enableChanged", this, SLOT(onEnableChanged(bool)))) {
        qDebug() << "BrightnessShortcut: QDBusConnection::sessionBus() connect enableChanged failed";
    }

    if (!QDBusConnection::sessionBus().connect(UKUI_SETTINGS_DAEMON_NAME,
                                               UKUI_GLOBAL_BRIGHTNESS_PATH,
                                               UKUI_SETTINGS_DAEMON_BRIGHTNESS_INTERFACE, "primaryBrightnessChangedEnd", this, SLOT(changeValue(quint32)))) {
        qDebug() << "BrightnessShortcut: QDBusConnection::sessionBus() connect primaryBrightnessChangedEnd failed";
    }

    if (!QDBusConnection::sessionBus().connect(UKUI_SETTINGS_DAEMON_NAME,
                                               UKUI_GLOBAL_BRIGHTNESS_PATH,
                                               UKUI_SETTINGS_DAEMON_BRIGHTNESS_INTERFACE, "enhancementBrightnessSwitchChanged", this, SLOT(onEnhanceChanged(bool)))) {
        qDebug() << "BrightnessShortcut: QDBusConnection::sessionBus() connect enhancementBrightnessSwitchChanged failed";
    }
}

void BrightnessShortcut::onEnableChanged(bool enable)
{
    if (m_isEnable == enable) {
        return;
    }

    m_isEnable = enable;
    if (m_isEnable) {
        QDBusReply<quint32> v = m_interface->call("getPrimaryBrightness");
        changeValue(v.isValid() ? v.value() : 0);
    }

    Q_EMIT enableStatusChanged(m_isEnable);
}

void BrightnessShortcut::onEnhanceChanged(bool enhance)
{
    if (enhance != m_enhance) {
        m_enhance = enhance;
        setMaxValue();
    }
}

BrightnessShortcutPlugin::BrightnessShortcutPlugin(QObject *parent) : UkuiShortcutPlugin(parent)
{

}

QStringList BrightnessShortcutPlugin::translations()
{
    return {QStringLiteral("brightness-shortcut")};
}

Shortcut *BrightnessShortcutPlugin::createShortcut()
{
    return new BrightnessShortcut;
}
