/*
 * Copyright (C) 2025, KylinSoft Co., Ltd.
 *
 *  This program is free software: you can redistribute it and/or modify
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
 * Authors: maomao <baijunjie@kylinos.cn>
 */


#include "security-control-config.h"
#include <function-control.h>
#include <QJsonObject>

using namespace UkuiNotification;
using namespace UkuiQuick;

static const QString CONFIG_NAME = "ukui-notification-v11";
SecurityControlConfig &SecurityControlConfig::instance()
{
    static SecurityControlConfig instance;
    return instance;
}

bool SecurityControlConfig::isEnabledAllNotification() const
{
    return !m_disabledAllNotification;
}

std::optional<bool> SecurityControlConfig::isNotificationAllowed(const QString& desktopEntry) const
{
    if (const auto& control = m_controlConfigMap.value(SettingsProperty::AllowNotify)) {
        return control->isAppAllowed(desktopEntry);
    }
    return std::nullopt;
}

bool SecurityControlConfig::isNotificationSwitchEditable(const QString& desktopEntry) const
{
    if (const auto& control = m_controlConfigMap.value(SettingsProperty::AllowNotify)) {
        return control->isOperationAllowed(desktopEntry);
    }
    return true;
}

std::optional<bool> SecurityControlConfig::isShowNotificationOnLockScreenAllowed(const QString& desktopEntry) const
{
    if (const auto& control = m_controlConfigMap.value(SettingsProperty::ShowNotificationOnLockScreen)) {
        return control->isAppAllowed(desktopEntry);
    }
    return std::nullopt;
}

bool SecurityControlConfig::isShowNotificationSwitchOnLockScreenEditable(const QString& desktopEntry) const
{
    if (const auto& control = m_controlConfigMap.value(SettingsProperty::ShowNotificationOnLockScreen)) {
        return control->isOperationAllowed(desktopEntry);
    }
    return true;
}

QStringList SecurityControlConfig::getAppList() const
{
    if (m_controlConfigMap.isEmpty()) {
        return {};
    }
    QStringList desktopEntries;
    for (const auto& control : m_controlConfigMap) {
        desktopEntries.append(control->appList());
    }
    return QSet<QString>(desktopEntries.begin(), desktopEntries.end()).values();;
}

SecurityControlConfig::SecurityControlConfig(QObject *parent) : QObject(parent)
{
    FunctionControl::instance()->addWatch(CONFIG_NAME);
    updateControlConfig(CONFIG_NAME);
    connect(FunctionControl::instance(), &FunctionControl::functionControlChanged, this, &SecurityControlConfig::onControlConfigChanged);
    connect(FunctionControl::instance(), &FunctionControl::functionControlExit, this, &SecurityControlConfig::onControlExited);
}

void SecurityControlConfig::onControlExited() {
    m_controlConfigMap.clear();
    m_disabledAllNotification = false;
    Q_EMIT exitControl();
}

void SecurityControlConfig::onControlConfigChanged(const QString &name)
{
    updateControlConfig(name);
}

void SecurityControlConfig::updateControlConfig(const QString &name)
{
    if (name == CONFIG_NAME) {
        const QJsonObject config = FunctionControl::instance()->functionControlObject(CONFIG_NAME).value(CONFIG_NAME).toObject();
        if (config.isEmpty()) {
            qWarning() << "SecurityControlConfig::updateControlConfig: config is empty";
            m_controlConfigMap.clear();
            m_disabledAllNotification = false;
            return;
        }
        const auto processConfig = [this, &config](const QString& key) {
            if (m_configKeyToPropertyMap.value(key) == SettingsProperty::ReceiveNotificationsFromApps) {
                m_disabledAllNotification = config.value(key).toBool();
            } else {
                const QJsonObject& configData = config.value(key).toObject();
                if (const auto& control = m_controlConfigMap.value(m_configKeyToPropertyMap.value(key))) {
                    control->updateConfig(configData);
                } else {
                    m_controlConfigMap[m_configKeyToPropertyMap.value(key)] = std::make_shared<ControlConfig>(key, configData);
                }
            }
        };

        for (const auto& key : config.keys()) {
            if (m_configKeyToPropertyMap.contains(key)) {
                processConfig(key);
                Q_EMIT configChanged(m_configKeyToPropertyMap.value(key));
            }
        }
    }
}